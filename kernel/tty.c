#include "type.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "proto.h"
#include "keyboard.h"
#include "x86.h"
#include "color.h"
#include "sheet.h"
#include "window.h"
#include "vga.h"
#include "bga.h"
int current_console; // 当前显示在屏幕上的console
void tty_write(TTY *tty, char *buf, int len);
int tty_read(TTY *tty, char *buf, int len);
extern MY_WINDOW* mywin2;
extern MY_WINDOW* mywin;
int current_window;
static void init_tty(TTY *tty);
static void tty_mouse(TTY *tty);
static void tty_dev_read(TTY *tty);
static void tty_dev_write(TTY *tty);
static void put_key(TTY *tty, u32 key);

void in_process(TTY *p_tty, u32 key)
{
	int real_line = p_tty->console->orig / SCR_WIDTH;
	// current_window = 0;
	if (!(key & FLAG_EXT))
	{
		put_key(p_tty, key);
	}
	else
	{
		int raw_code = key & MASK_RAW;
		switch (raw_code)
		{
		case ENTER:
			put_key(p_tty, '\n');
			p_tty->status = p_tty->status & 3; //&3'b011
			// current_window=1;
			break;
		case BACKSPACE:
			put_key(p_tty, '\b');
			break;
		case UP:
			if (p_tty->console->current_line < 43)
			{
				disable_int();
				p_tty->console->current_line++;
				outb(CRTC_ADDR_REG, START_ADDR_H);
				outb(CRTC_DATA_REG, ((80 * (p_tty->console->current_line + real_line)) >> 8) & 0xFF);
				outb(CRTC_ADDR_REG, START_ADDR_L);
				outb(CRTC_DATA_REG, (80 * (p_tty->console->current_line + real_line)) & 0xFF);
				enable_int();
			}
			break;
		case DOWN:
			if (p_tty->console->current_line > 0)
			{
				disable_int();
				p_tty->console->current_line--;
				outb(CRTC_ADDR_REG, START_ADDR_H);
				outb(CRTC_DATA_REG, ((80 * (p_tty->console->current_line + real_line)) >> 8) & 0xFF);
				outb(CRTC_ADDR_REG, START_ADDR_L);
				outb(CRTC_DATA_REG, (80 * (p_tty->console->current_line + real_line)) & 0xFF);
				enable_int();
			}
			break;
		case F1:
			// "llll"
			disable_int();
			current_window = 1;
			enable_int();
			break;	
		case F2:
			//"hello,my gui"
			disable_int();
			current_window = 2;	
			enable_int();
			break;
		case F3:

		case F4:
		case F5:
		case F6:
		case F7:
		case F8:
		case F9:
		case F10:
		case F11:
		case F12:
			select_console(raw_code - F1);
			break;
		}
	}
}

#define TTY_FIRST (tty_table)
#define TTY_END (tty_table + NR_CONSOLES)

void task_tty()
{
	//bga_ioctl(BGA_DISABLE,0);
	TTY *p_tty;
	for (p_tty = TTY_FIRST; p_tty < TTY_END; p_tty++)
	{
		init_tty(p_tty);
	}
	p_tty = tty_table;
	// win_cmd_put_string(mywin,"dddddddd");
	// win_cmd_put_string(mywin,"dsfdf");,         不能调用
	// win_cmd_put_string(mywin2,"dsfdf");
	sys_gui(p_tty);

	//while (1);
	//win_cmd_put_string(mywin2,"hello,my gui");
	// win_cmd_put_string(mywin,"dsfdf");
	// while(1);
	select_console(0);

	//设置第一个tty光标位置，第一个tty需要特殊处理
	disable_int();
	outb(CRTC_ADDR_REG, CURSOR_H);
	outb(CRTC_DATA_REG, ((disp_pos / 2) >> 8) & 0xFF);
	outb(CRTC_ADDR_REG, CURSOR_L);
	outb(CRTC_DATA_REG, (disp_pos / 2) & 0xFF);
	enable_int();

	//轮询
	while (1)
	{
		for (p_tty = TTY_FIRST; p_tty < TTY_END; p_tty++)
		{
			do
			{
				sheet_refresh_rect(sheets);
				// sheet_change_output(sheets);

				tty_mouse(p_tty); /* tty判断鼠标操作 */
				tty_dev_read(p_tty);  /* 从键盘输入缓冲区读到这个tty自己的缓冲区 */
				tty_dev_write(p_tty); /* 把tty缓存区的数据写到这个tty占有的显存 */

			} while (p_tty->ibuf_cnt);
		}
	}
}

static void init_tty(TTY *p_tty)
{
	p_tty->ibuf_read_cnt = p_tty->ibuf_cnt = 0;
	p_tty->status = TTY_STATE_DISPLAY;
	p_tty->ibuf_read = p_tty->ibuf_head = p_tty->ibuf_tail = p_tty->ibuf;
	int det = p_tty - tty_table;
	p_tty->console = console_table + det;

	p_tty->mouse_left_button = 0;
	p_tty->mouse_mid_button = 0;
	p_tty->mouse_X = 0;
	p_tty->mouse_Y = 0;
	init_screen(p_tty);
}

static void tty_mouse(TTY *tty)
{
	if (is_current_console(tty->console))
	{
		int real_line = tty->console->orig / SCR_WIDTH;
		if (tty->mouse_left_button)
		{

			if (tty->mouse_Y > MOUSE_UPDOWN_BOUND)
			{ //按住鼠标左键向上滚动
				if (tty->console->current_line < 43)
				{
					disable_int();
					tty->console->current_line++;
					outb(CRTC_ADDR_REG, START_ADDR_H);
					outb(CRTC_DATA_REG, ((80 * (tty->console->current_line + real_line)) >> 8) & 0xFF);
					outb(CRTC_ADDR_REG, START_ADDR_L);
					outb(CRTC_DATA_REG, (80 * (tty->console->current_line + real_line)) & 0xFF);
					enable_int();
					tty->mouse_Y = 0;
				}
			}
			else if (tty->mouse_Y < -MOUSE_UPDOWN_BOUND)
			{ //按住鼠标左键向下滚动
				if (tty->console->current_line > 0)
				{
					disable_int();
					tty->console->current_line--;
					outb(CRTC_ADDR_REG, START_ADDR_H);
					outb(CRTC_DATA_REG, ((80 * (tty->console->current_line + real_line)) >> 8) & 0xFF);
					outb(CRTC_ADDR_REG, START_ADDR_L);
					outb(CRTC_DATA_REG, (80 * (tty->console->current_line + real_line)) & 0xFF);
					enable_int();
					tty->mouse_Y = 0;
				}
			}
		}

		if (tty->mouse_mid_button)
		{ //点击中键复原
			disable_int();
			tty->console->current_line = 0;
			outb(CRTC_ADDR_REG, START_ADDR_H);
			outb(CRTC_DATA_REG, ((80 * (tty->console->current_line + real_line)) >> 8) & 0xFF);
			outb(CRTC_ADDR_REG, START_ADDR_L);
			outb(CRTC_DATA_REG, (80 * (tty->console->current_line + real_line)) & 0xFF);
			enable_int();
			tty->mouse_Y = 0;
		}
	}
}

static void tty_dev_read(TTY *tty)
{
	if (is_current_console(tty->console))
	{
		keyboard_read(tty);
	}
}

static void tty_dev_write(TTY *tty)
{
	
	if (tty->ibuf_cnt)
	{
		char ch = *(tty->ibuf_tail);
		tty->ibuf_tail++;
		if (tty->ibuf_tail == tty->ibuf + TTY_IN_BYTES)
		{
			tty->ibuf_tail = tty->ibuf;
		}
		tty->ibuf_cnt--;

		if (ch == '\b')
		{
			if (tty->ibuf_read_cnt == 1)
			{
				tty->ibuf_read_cnt--;
				tty->ibuf_head--;
				tty->ibuf_tail--;
				return;
			}
			else
			{
				tty->ibuf_read_cnt -= 2;
				if (tty->ibuf_head == tty->ibuf)
				{
					tty->ibuf_head = tty->ibuf_tail = &tty->ibuf[256 - 2];
				}
				else
				{
					tty->ibuf_head--;
					tty->ibuf_tail--;
					tty->ibuf_head--;
					tty->ibuf_tail--;
				}
			}
		}
		// //out_char(tty->console, ch);
		// if(current_window==1){
		// 	win_cmd_put_char(mywin, ch);
		// }
		// else if(current_window==2){
		// 	win_cmd_put_char(mywin2, ch);
		// }
		
		if(current_window==2){
			win_cmd_put_char(mywin2, ch);
		}
		if(current_window==1){
			win_cmd_put_char(mywin, ch);
		}
		// current_window=-1;
		// win_cmd_put_string(mywin,"dsfsdfsdfsdf");  无效
		// win_cmd_put_char(mywin2,ch); 有效
	}
}

static void put_key(TTY *tty, u32 key)
{
	if (tty->ibuf_cnt < TTY_IN_BYTES)
	{
		*(tty->ibuf_head) = key;
		tty->ibuf_head++;
		if (tty->ibuf_head == tty->ibuf + TTY_IN_BYTES)
			tty->ibuf_head = tty->ibuf;
		tty->ibuf_cnt++;
		tty->ibuf_read_cnt++;
	}
}






/*****************************************************************************
 *                                tty_write
 ****************************************************************************

 *  当fd=STD_OUT时，write()系统调用转发到此函数
 *****************************************************************************/
void tty_write(TTY *tty, char *buf, int len)
{
	while (--len >= 0)

		out_char(tty->console, *buf++);
		//win_cmd_put_char(mywin2,*buf++);
}

/*****************************************************************************
 *                                tty_read
 ****************************************************************************

 *  当fd=STD_IN时，read()系统调用转发到此函数
 *****************************************************************************/
int tty_read(TTY *tty, char *buf, int len)
{
	int i = 0;
	if (!tty->ibuf_read_cnt)
	{
		tty->status |= TTY_STATE_WAIT_ENTER;
	}

	while ((tty->status & TTY_STATE_WAIT_ENTER))
		;

	while (tty->ibuf_read_cnt && i < len)
	{
		buf[i] = *tty->ibuf_read;
		tty->ibuf_read++;
		if (tty->ibuf_read == tty->ibuf + TTY_IN_BYTES)
		{
			tty->ibuf_read = tty->ibuf;
		}
		tty->ibuf_read_cnt--;
		i++;
	}

	return i;
}