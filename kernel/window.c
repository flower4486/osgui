#include "window.h"
#include "type.h"
#include "const.h"
#include "string.h"
#include "protect.h"
#include "tty.h"
#include "proc.h"
#include "global.h"
#include "proto.h"
#include "sheet.h"
#include "color.h"
#include "vga.h"
#include "gui.h"

//vga.c
extern u32 * lfb_vid_memory;
extern u16 lfb_resolution_x;
extern u16 lfb_resolution_y;
extern u16 lfb_resolution_b;
//font.h included in vga.c
extern u8 number_font[][12];
extern u8 term_cursor_font[][12];


//list of windows
MY_WINDOW * mywin_list_header = NULL;
MY_WINDOW * mywin_list_end = NULL;
MY_WINDOW * mywin_list_kbd_input = NULL;


// void win_test()
// {
// 	struct sheets* s=sheets;
// 	MY_WINDOW* mywin=alloc_window();
// 	init_window(mywin);
// 	draw_win_rect(mywin);

// 	MY_WINDOW* mywin2=alloc_window();
// 	// init_window(mywin2);
// 	// draw_win_rect(mywin2);
// 	sheet_slide(sheets,mywin2->sheet,100,100);

// 	// win_cmd_put_string(mywin2,"hello,my gui");
// 	// vga_draw_mouse(0,0,Red,Blue);
// 	drawmouse(0,0);
// }

MY_WINDOW* alloc_window()
{
	MY_WINDOW* mywin=(MY_WINDOW*)sys_malloc(sizeof(MY_WINDOW));
	memset((u8*)mywin,0,sizeof(MY_WINDOW));

	mywin->sheet=sheet_alloc(sheets);
 
	if(mywin_list_header == NULL)
	{
		mywin_list_header = mywin;
		mywin_list_end = mywin;
	}
	else
	{
		MY_WINDOW * tmpwin = mywin_list_header;
		for(;;)
		{
			if(tmpwin->nxt == NULL)
			{
				tmpwin->nxt = mywin;
				mywin->pre = tmpwin;
				break;
			}
			else
			{
				tmpwin = tmpwin->nxt;
			}
		}
		mywin_list_end = mywin;
	}

	return mywin;
}

void init_window(MY_WINDOW* mywin)
{
	sheet_setsheet(mywin->sheet,200,100,20,20);
	u8* winbuf=(u8*)sys_malloc(mywin->sheet->width*mywin->sheet->height);

	sheet_setbuf(mywin->sheet,winbuf);
	sheet_set_layer(sheets,mywin->sheet,100);

	mywin->title_rect.height=VGA_CHAR_HEIGHT;
	mywin->title_rect.width=0.9*mywin->sheet->width;
	mywin->title_rect.rx=0;
	mywin->title_rect.ry=0;
	mywin->title_rect.color=Yellow;

	mywin->close_btn_rect.height=VGA_CHAR_HEIGHT;
	mywin->close_btn_rect.width=0.1*mywin->sheet->width;
	mywin->close_btn_rect.rx=0.9*mywin->sheet->width;
	mywin->close_btn_rect.ry=0;
	mywin->close_btn_rect.color=Red;

	mywin->cmd_rect.height=mywin->sheet->height-12;
	mywin->cmd_rect.width=mywin->sheet->width;
	mywin->cmd_rect.rx=0;
	mywin->cmd_rect.ry=12;
	mywin->cmd_rect.color=Blue;

	mywin->cmd_font_color=White;

	mywin->cmd_cursor_x=0;
	mywin->cmd_cursor_y=0;
	mywin->cmd_font_height=VGA_CHAR_HEIGHT;
	mywin->cmd_font_width=VGA_CHAR_WIDTH;
	return;
}

void draw_win_rect(MY_WINDOW* mywin)
{
	struct sheets* s=sheets;
	u8* buf=mywin->sheet->buf;
	int i,j;
	for ( i = 0; i < mywin->title_rect.height; i++)
	{
		for (j = 0; j < mywin->title_rect.width; j++)
		{
			buf[(i+mywin->title_rect.ry)*mywin->sheet->width+j+mywin->title_rect.rx]=mywin->title_rect.color;
		}
	}


	for ( i = 0; i < mywin->cmd_rect.height; i++)
	{
		for (j = 0; j < mywin->cmd_rect.width; j++)
		{
			buf[(i+mywin->cmd_rect.ry)*mywin->sheet->width+j+mywin->cmd_rect.rx]=mywin->cmd_rect.color;
		}
	}


	for ( i = 0; i < mywin->close_btn_rect.height; i++)
	{
		for (j = 0; j < mywin->close_btn_rect.width; j++)
		{
			buf[(i+mywin->close_btn_rect.ry)*mywin->sheet->width+j+mywin->close_btn_rect.rx]=mywin->close_btn_rect.color;
		}
	}
}

void win_cmd_put_char(MY_WINDOW* mywin,u8 ahcar)
{
	win_sheet_put_char(mywin,mywin->cmd_cursor_x,mywin->cmd_rect.ry+mywin->cmd_cursor_y,ahcar,mywin->cmd_font_color,Blue);
	//vga_write_char(mywin->sheet->x+mywin->cmd_cursor_x,mywin->cmd_rect.ry+mywin->sheet->y+mywin->cmd_cursor_y,ahcar,mywin->cmd_font_color,Blue);
	mywin->cmd_cursor_x+=VGA_CHAR_WIDTH;//+cursor_side;
	if (mywin->cmd_cursor_x%mywin->sheet->width==0)
	{
		mywin->cmd_cursor_x=0;
		mywin->cmd_cursor_y+=VGA_CHAR_HEIGHT;
	}
	sheet_refresh_rect(sheets);
	return;
}

void win_cmd_put_string(MY_WINDOW* mywin,char* s)
{
	while (*s!='\0')
	{
		win_cmd_put_char(mywin,*(s++));
	}
	return;
}

// void move_win_mouse(MY_WINDOW* mywin)
// {
// 	vga_draw_mouse(0,0,Blue);
// 	while(1);

// }
void drawmouse(int x,int y){ /* 画鼠标*/
   static char cursor[12][12] = {
		"************",
		"*OOOOOOOOOO*",
		"*OOOOOOOOO*.",
		"*OOOOOOOO*..",
		"*OOOOOOOO*..",
		"*OOOOOOO*...",
		"*OOOOOOO*...",
		"*OOOOOOOO*..",
		"*OOOO**OOO*.",
		"*OOO*..*OOO*",
		"*OO*....*OO*",
		"*O*......***"
	};
   for (int j  = y; j < y+12; j++) {
		for (int i = x; i < x+12; i++) {
			if (cursor[j-y][i-x] == '*') {
				// mouse[y * 16 + x] = COL8_000000;
            putPoint(i, j, Black);
			}
			if (cursor[j-y][i-x] == 'O') {
				// mouse[y * 16 + x] = COL8_FFFFFF;
            putPoint(i, j, White);
			}
			// if (cursor[j-y][i-x] == '.') {
			// 	// mouse[y * 16 + x] = back_color;
            // putPoint(i,j,back_color);
			// }
		}
   }
}

