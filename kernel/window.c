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


//font.h included in vga.c
extern u8 number_font[][12];
extern u8 term_cursor_font[][12];

//list of windows
MY_WINDOW * mywin_list_header = NULL;
MY_WINDOW * mywin_list_end = NULL;

MY_WINDOW *mywin,*mywin2;

extern struct sheet* mouse_bind_sheet;

static void init_window(MY_WINDOW* mywin)
{
	sheet_setsheet(mywin->sheet,120,86,vga_screen_width/10,vga_screnn_height/10);
	 u32* winbuf=(u32*)(K_PHY2LIN(sys_kmalloc(mywin->sheet->width*mywin->sheet->height*4)));
	fastset(winbuf,0,mywin->sheet->width*mywin->sheet->height);
	 sheet_setbuf(mywin->sheet,winbuf);
	sheet_set_layer(sheets,mywin->sheet,100);
	// 画窗口
	mywin->title_rect.height=VGA_CHAR_HEIGHT;
	mywin->title_rect.width=0.9*mywin->sheet->width;
	mywin->title_rect.rx=0;
	mywin->title_rect.ry=0;
	mywin->title_rect.color=rgb_Yellow;

	mywin->close_btn_rect.height=VGA_CHAR_HEIGHT;
	mywin->close_btn_rect.width=0.1*mywin->sheet->width;
	mywin->close_btn_rect.rx=0.9*mywin->sheet->width;
	mywin->close_btn_rect.ry=0;
	mywin->close_btn_rect.color=rgb_Red;

	mywin->cmd_rect.height=mywin->sheet->height-VGA_CHAR_HEIGHT;
	mywin->cmd_rect.width=mywin->sheet->width;
	mywin->cmd_rect.rx=0;
	mywin->cmd_rect.ry=VGA_CHAR_HEIGHT;
	mywin->cmd_rect.color=rgb_Blue;

	mywin->cmd_font_color=rgb_White;

	mywin->cmd_cursor_x=0;
	mywin->cmd_cursor_y=0;
	mywin->cmd_font_height=VGA_CHAR_HEIGHT;
	mywin->cmd_font_width=VGA_CHAR_WIDTH;
	return;
}

static void draw_win_rect(MY_WINDOW* mywin)
{
	struct sheets* s=sheets;
	u32* buf=mywin->sheet->buf;
	int i,j;
	for ( i = 0; i < mywin->title_rect.height; i++)
	{
		for (j = 0; j < mywin->title_rect.width; j++)
		{
			*(buf+(i+mywin->title_rect.ry)*mywin->sheet->width+j+mywin->title_rect.rx)=mywin->title_rect.color;
		}
	}


		for ( i = 0; i < mywin->cmd_rect.height; i++)
	{
		for (j = 0; j < mywin->cmd_rect.width; j++)
		{
			*(buf+(i+mywin->cmd_rect.ry)*mywin->sheet->width+j+mywin->cmd_rect.rx)=mywin->cmd_rect.color;
		}
	}


		for ( i = 0; i < mywin->close_btn_rect.height; i++)
	{
		for (j = 0; j < mywin->close_btn_rect.width; j++)
		{
			*(buf+(i+mywin->close_btn_rect.ry)*mywin->sheet->width+j+mywin->close_btn_rect.rx)=mywin->close_btn_rect.color;
		}
	}

	sheets->need_update=TRUE;
}

void win_test()
{
	
	mywin=alloc_window();
	mywin2=alloc_window();
	sheet_slide(sheets,mywin2->sheet,100,100);
	win_cmd_put_string(mywin2,"hello,my gui");
	win_cmd_put_string(mywin,"llll");
	mouse_bind_sheet=mywin2->sheet;

}

MY_WINDOW* alloc_window()
{
	MY_WINDOW* mywin=(MY_WINDOW*)K_PHY2LIN(sys_kmalloc(sizeof(MY_WINDOW)));
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
	init_window(mywin);
	draw_win_rect(mywin);
	return mywin;
}

void do_get_win()
{
	current_window=alloc_window();
	sheet_set_top(current_window->sheet);
	return;
}
void sys_get_win()
{
	return do_get_win();
}

void win_cmd_put_char(MY_WINDOW* mywin,u8 ahcar)
{
	if(mywin->cmd_cursor_x!=0&&mywin->cmd_cursor_x>=mywin->sheet->width)
	{
		if (mywin->cmd_cursor_y<=mywin->cmd_rect.height-2*VGA_CHAR_HEIGHT)
		{
			mywin->cmd_cursor_x=0;
			mywin->cmd_cursor_y+=VGA_CHAR_HEIGHT;
		}else{
			mywin->cmd_cursor_x-=VGA_CHAR_WIDTH;
		}
	}
	if (ahcar=='\b')
	{
		if (mywin->cmd_cursor_x!=0)
		{
			mywin->cmd_cursor_x-=VGA_CHAR_WIDTH;
		}else if (mywin->cmd_cursor_x==0&&mywin->cmd_cursor_y>0)
		{
			mywin->cmd_cursor_y-=VGA_CHAR_HEIGHT;
			mywin->cmd_cursor_x=mywin->sheet->width-VGA_CHAR_WIDTH;
		}
	win_sheet_put_char(mywin,mywin->cmd_cursor_x,mywin->cmd_rect.ry+mywin->cmd_cursor_y,' ',mywin->cmd_font_color,rgb_Blue);
	}else{
	win_sheet_put_char(mywin,mywin->cmd_cursor_x,mywin->cmd_rect.ry+mywin->cmd_cursor_y,ahcar,mywin->cmd_font_color,rgb_Blue);
	mywin->cmd_cursor_x+=VGA_CHAR_WIDTH;
	}
	sheets->need_update=TRUE;
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

void drawmouse(u32* buf)
{ /* 画鼠标*/
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
   for (int j  = 0; j < 12; j++) {
		for (int i = 0; i < 12; i++) {
			if (cursor[j][i] == '*') {
				*(buf+i+j*12)=rgb_Black;
			}
			if (cursor[j][i] == 'O') {
				*(buf+i+j*12)=rgb_White;
			}
		}
   }
}
void win_sheet_put_char(MY_WINDOW* mywin,int x,int y,int achar,u32 color,u32 bkcolor)
{
    if (achar > 128)
   {
      achar = 4;
   }
   u8 *c = number_font[achar];

   for (int i = 0; i < VGA_CHAR_HEIGHT; ++i)
   {
      for (int j = 0; j < VGA_CHAR_WIDTH; ++j)
      {

         if (c[i] & (1 << (8 - j)))
         {
           *(mywin->sheet->buf+j+x+mywin->sheet->width*(y+i))=color;
         }
         else
         {
            *(mywin->sheet->buf+j+x+mywin->sheet->width*(y+i))=bkcolor;
         }
      }
   }
	sheets->need_update=TRUE;

   return;
}
