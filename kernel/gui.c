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
#include "stdio.h"
#include "vga.h"
#include "color.h"
#include "font.h"
#include "window.h"
#include "sheet.h"
#include "gui.h"
#include "mouse.h"
#include "pci.h"
#include "bga.h"
void do_set_screen(int width,int height)
{
   gui_mode=1;
   bga_set_resolution(width, height);
   
   //fastset((u32*)vga_video_start,rgb_Blue,vga_screen_width*vga_screnn_height);
  // fastset((u32*)vga_video_start+320*200,rgb_Red,320*200);
  // fastset((u32*)vga_video_start,rgb_White,320*200);
   //bga_ioctl(BGA_SWAP_BUFFERS,0);
   init_gui();
   return;
}
void sys_set_screen(void *uesp)
{
    return do_set_screen(get_arg(uesp, 1), get_arg(uesp, 2));
}


void init_gui()
{
   
   #ifdef all_debug
   sheets=sheets_init();
   //set_bkcolor(sheets,rgb_Blue);

   sheet_mouse = sheet_alloc(sheets);
	sheet_setsheet(sheet_mouse, 12, 12, 100, 100);
	u32 *sheet_buf4 = (u32 *)K_PHY2LIN(sys_kmalloc(sheet_mouse->width * sheet_mouse->height*4));
	drawmouse(sheet_buf4);
	sheet_setbuf(sheet_mouse, sheet_buf4);
	sheet_set_layer(sheets,sheet_mouse,10000);
   #endif
   
   #ifdef gui_debug
   int n=100;
   while (n--)
   {
      putPoint(n+100,100,rgb_Red);
   }
   rectangle(0,0,100,100,rgb_Blue);
   drawline(200,10,130,rgb_White);
   vga_write_char(100,100,'h',rgb_Red,rgb_Black);
   vga_write_char(108,100,'e',rgb_Red,rgb_Black);
   vga_write_char(116,100,'l',rgb_Red,rgb_Black);
   vga_write_char(124,100,'l',rgb_Red,rgb_Black);
   vga_write_char(132,100,'o',rgb_Red,rgb_Black);
   vga_write_char(140,100,',',rgb_Red,rgb_Black);
   vga_write_string(148,100,"world",rgb_Red,rgb_Black);
   #endif
   
   #ifdef sheet_debug
   sheets=sheets_init();
   //set_bkcolor(sheets,rgb_Black);
   sheet_test();
   #endif
  
   #ifdef win_debug
   sheets=sheets_init();
   set_bkcolor(sheets,rgb_Black);
   win_test();
   #endif

}

void putPoint(int x, int y, int Color) /* 画点函数 */
{
   u32 *p;
   p = (u32*)vga_video_start;
   *(u32*)(x + y * 320 + p) = Color;
}

void drawline(int row, int start, int end, int color)
// draw a line form x to y
{
   u32 *lstart, *lend;
   lstart = (u32 *)(vga_video_start + row * vga_screen_width + start);

   fastset((void *)lstart, color, end - start);
}
void rectangle(int x1, int y1, int x2, int y2, int Color) /* 画一矩形*/
{
   for (int i = x1; i < x2; i++)
   {
      for (int j = y1; j < y2; j++)
      {
         putPoint(i, j, Color);
      }
   }
}

 void vga_write_char(int x, int y, int val, u32 color, u32 backColour)
{
   if (val > 128)
   {
      val = 4;
   }
   u8 *c = number_font[val];

   for (int i = 0; i < VGA_CHAR_HEIGHT; ++i)
   {
      for (int j = 0; j < VGA_CHAR_WIDTH; ++j)
      {

         if (c[i] & (1 << (8 - j)))
         {
            putPoint(x + j, y + i, color);
         }
         else
         {
            putPoint(x + j, y + i, backColour);
         }
      }
   }
   return;
}


 void vga_write_string(int x, int y, char *s, u32 color, u32 backcolor)
{
   while (*s != '\0')
   {
      vga_write_char(x, y, (int)(*(s++)), color, backcolor);
      x += VGA_CHAR_WIDTH;
   }
}


 void vga_draw_mouse(int x, int y, u32 color, u32 backColour)
{

   u8 *c = term_cursor_font[0];

   for (int i = 0; i < VGA_CHAR_HEIGHT; ++i)
   {
      for (int j = 0; j < VGA_CHAR_WIDTH; ++j)
      {

         if (c[i] & (1 << (8 - j)))
         {
            putPoint(x + j, y + i, color);
         }
         else
         {
            putPoint(x + j, y + i, backColour);
         }
      }
   }
   return;
}

int draw_rect(u8 * buffer, int color, int x, int y, int w, int h)
{
   int wd_in_bytes;
	int off, i;
		wd_in_bytes = 320;
		for(i=0;i < h;i++)
		{
			off = wd_in_bytes * (y + i) + x;
			memset((buffer + off), color, w);
		}
	return 0;
}