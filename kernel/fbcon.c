#include "type.h"
#include "protect.h"
#include "proc.h"
#include "tty.h"
#include "const.h"
#include "global.h"
#include "proto.h"
#include "bga.h"
#include "console.h"
#include "keyboard.h"
#include "memman.h"
#include "spinlock.h"
#include "stdio.h"
#include "string.h"
#include "x86.h"
#include "fs.h" //added by mingxuan 2019-5-19
#include "vfs.h"


static int cur_col = 0;
static int cur_row = 0;
static int _fnt_char_width = 0;
static int _fnt_char_height = 0;
static int _scr_max_rows = 0;
static int _scr_max_cols = 0;
static int _fbbufsize = 0;
static int _fbwidth = 0;
static int _fbheight = 0;
static int _fbpixels_per_row = 0;
static int _fbscale = 1;
static int cursor_blink = 0;
static u32 _basecolor = 0x0;
static uintptr_t _fb_paddr = 0;
static u32* _fb = NULL;
static u32* cur_fb = NULL;
static volatile int framecnt = 0;
static u16* textbuf;
char* hzk16h_buf;
volatile u32 buflock;
static int halfchar;

#define UNIT_CHAR_H (_fnt_char_height * _fbscale)
#define UNIT_CHAR_W (_fnt_char_width * _fbscale)
#define INDEX(row, col, mx) ((row) * (mx) + (col))
#define HZK16_SIZE (261696)
#include "font8x8.h"
static void fast_copy(void* dst, void* src, int len);

static void fbcon_draw_raw(int row, int col, char ch) {
    for (int x = 0; x < UNIT_CHAR_H; x++) {
        for (int y = 0; y < UNIT_CHAR_W; y++) {
            u32 ind = (col * UNIT_CHAR_W + x) + ((row * UNIT_CHAR_H + y) * _fbpixels_per_row);
            u32 clr = _basecolor;
            if (font8x8_basic[ch & 0x7f][y / _fbscale] &
                (1 << (x / _fbscale))) {
                clr ^= 0xffffffff;
            }
            cur_fb[ind] = clr;
        }
    }
}

static void fbcon_draw_hzk(int row, int col, u16 ch) {
    u8 code0 = ((ch >> 8) & 0xff);
    u8 code1 = (ch & 0xff);
    if (code0 < 0xa0) {fbcon_draw_raw(row, col, code1); return; }
    else { code0 -= 0xa0; code1 -= 0xa0; }
    u32 offset = (94 * (code0 - 1) + (code1 - 1)) * 32;
    u16* hzk16h = (u16*)&hzk16h_buf[offset];
    for (int x = 0; x < UNIT_CHAR_H; x++) {
        for (int y = 0; y < UNIT_CHAR_W; y++) {
            u32 ind = (col * UNIT_CHAR_W + (UNIT_CHAR_W - x)) + ((row * UNIT_CHAR_H + y) * _fbpixels_per_row);
            u32 clr = _basecolor;
            u16 bigend = (hzk16h[y] >> 8) | ((hzk16h[y] & 0xff) << 8);
            if (bigend & (1 << (x))) {
                clr ^= 0xffffffff;
            }
            // if ((u32)cur_fb + ind * 4 == 0xfd600000) {
            //     kprintf("%x %d %d %d %d\n", ch, col, row, x, y);
            // }
            cur_fb[ind] = clr;
        }
    }
}

static void draw_cursor() {
    fbcon_draw_raw(cur_row, cur_col, 0);
}

static void draw_cursor_clear() {
    fbcon_draw_raw(cur_row, cur_col, 1);
}

static void fbcon_scroll() {
    int lineoffset = _fbwidth * _fnt_char_height * _fbscale * sizeof(*textbuf);
    fast_copy(  (void*)cur_fb, 
                (void*)cur_fb + lineoffset, 
                _fbbufsize - lineoffset);
    memset((void*)cur_fb + _fbbufsize - lineoffset, 0, lineoffset);
}

static void textbuf_scroll() {
    memcpy((void*) textbuf, (void*)textbuf + _scr_max_cols *2, (_scr_max_rows-1)*_scr_max_cols * 2);
    memset((void*)textbuf + (_scr_max_rows-1)*_scr_max_cols*2, ' ', _scr_max_cols*2);
}

static void fbcon_putchar(u8 ch) {
    if (ch >= 0xa0) {
        if (halfchar == 0) {
            textbuf[INDEX(cur_row, cur_col, _scr_max_cols)] = ch << 8;
            halfchar = 1;
            return;
        }
    }
    switch (ch)
	{
	case '\n':
		cur_row ++;
		cur_col = 0;
		if (cur_row >= _scr_max_rows) {
			textbuf_scroll();
			cur_row --;
		}
		break;
	case '\r':
		cur_col = 0;
		break;
	case '\b':
		if (cur_col == 0) {
			if (cur_row) cur_row --;
			cur_col = _scr_max_cols - 1;
		}
		else {
			cur_col --;
		}
		break;
	default:
        if (halfchar) {
            halfchar = 0;
		    textbuf[INDEX(cur_row, cur_col, _scr_max_cols)] |= ch;
        }
        else {
		    textbuf[INDEX(cur_row, cur_col, _scr_max_cols)] = ch;
        }
		cur_col ++;
		if (cur_col >= _scr_max_cols) {
			cur_col = 0;
			cur_row ++;
		}

		if (cur_row >= _scr_max_rows) {
			textbuf_scroll();
			cur_row --;
		}
		break;
	}
}

static void fbcon_clear_screen() {
    for (int i = 0; i < _scr_max_cols * _scr_max_rows; ++ i) {
        textbuf[i] = ' ';
    }
}

void fbcon_screen_setup() {
    cur_col = 0;
    cur_row = 0;
    _fnt_char_width = 8;
    _fnt_char_height = 8;
    _fb = (u32*)bga_ioctl(BGA_GET_BUFFER, 0);
    _fb_paddr = bga_ioctl(BGA_GET_BUFFER, 0);
    _fbwidth = bga_ioctl(BGA_GET_WIDTH, 0);
    _fbheight = bga_ioctl(BGA_GET_HEIGHT, 0);
    _fbpixels_per_row = _fbwidth;
    _fbbufsize = _fbwidth * _fbheight * 4;
    _fbscale = 2;
    cur_fb = _fb;
    _scr_max_cols = _fbwidth / _fnt_char_width / _fbscale;
    _scr_max_rows = _fbheight / _fnt_char_height / _fbscale;

    _basecolor = 0;
    cursor_blink = 1;
    framecnt = 0;
    kprintf("allocating screen res for %dx%d...\n", _scr_max_rows, _scr_max_cols);
    textbuf = (u16*)K_PHY2LIN(sys_kmalloc(_scr_max_cols * _scr_max_rows * sizeof(*textbuf))); // alloc double for later use
    fbcon_clear_screen();
    // memset(textbuf, ' ', _scr_max_cols * _scr_max_rows);
    hzk16h_buf = (char*)K_PHY2LIN(sys_kmalloc(HZK16_SIZE));
    halfchar = 0;
    kprintf("bufsz = %p, maxsize(%d, %d) textbuf=%p, fontbuf=%p\n", _fbbufsize, _scr_max_rows, _scr_max_cols, textbuf, hzk16h_buf);
}


static void do_fbcon_write(char* buf, int nr) {
    disable_int();
    while (nr--) {
        fbcon_putchar(*buf++);
    }
    enable_int();
}

void fbcon_tty_write(TTY* tty, char ch) {
    do_fbcon_write(&ch, 1);
}

static void blink_ctrl() {
    if (framecnt < 10) {
        framecnt ++;
        // kprintf("frame %d\n", framecnt);
    }
    else {
        framecnt = 0;
        cursor_blink = ! cursor_blink;
        // kprintf("should change\n");
    }
}

static void fast_copy(void* dst, void* src, int len) {
    // assume all address align, 32 bytes a time
    u32* _src = src;
    u32* _dst = dst;
    int i;
    for (i = 0; i < len / 4; i += 8) {
        _dst[i + 0] = _src[i + 0];
        _dst[i + 1] = _src[i + 1];
        _dst[i + 2] = _src[i + 2];
        _dst[i + 3] = _src[i + 3];
        _dst[i + 4] = _src[i + 4];
        _dst[i + 5] = _src[i + 5];
        _dst[i + 6] = _src[i + 6];
        _dst[i + 7] = _src[i + 7];
    }
}

static void textbuf_render() {
    for (int row = 0; row < _scr_max_rows; ++ row) {
        for (int col = 0; col < _scr_max_cols; ++ col) {
            fbcon_draw_hzk(row, col, textbuf[INDEX(row, col, _scr_max_cols)]);
        }
    }
}

void fbcon_switchfont() {
    while (xchg(&buflock, 1) == 1)
        yield();
    _fbscale = 1;
    xchg(&buflock, 0);
}

void _tty() 
{
   // #ifdef USE_FBCON
    // main rountine for drawing framebuffered console
    //mmap((u32)_fb, _fb_paddr, _fbwidth * _fbheight * 4 * 2);
    buflock = 0;

    // fbcon_write(text1, strlen(text1));
    int cur_buf = 0, last_buf = 1;
    while (1) {
        // fast_copy((void*)_fb + cur_buf * _fbbufsize, (void*)_fb + last_buf * _fbbufsize, _fbbufsize);
        draw_cursor_clear();
        while (xchg(&buflock, 1) == 1)
            yield();
        textbuf_render();
        xchg(&buflock, 0);
        blink_ctrl();
        for (int i = 0; i < 3; i++)
        {
            fbcon_putchar('b');
        }
        
       
        if (cursor_blink)
            draw_cursor();
        bga_ioctl(BGA_SWAP_BUFFERS, cur_buf);
        last_buf = cur_buf;
        cur_buf = !cur_buf;
        cur_fb = (void*)_fb + cur_buf * _fbbufsize;

        sleep(5); // to no need to refresh that fast
    }
   // #else 
    //while(1) yield();
  // #endif
}