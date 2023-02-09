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
#

void sys_gui(){
	do_gui();
   return;
}
void do_gui()
{
   // vga_write_regs(vga_320x200x256);
 //  open gui mode
   //vga_write_regs(vga_80x25_text);
   
   //int p;
   //u8* p=(u8 *)memstart;
   //if(sheets==NULL)
	
   //Paint the screen black
   //memset(p,Black,memsize);
  
   //disable_int();
     //bga_set_resolution(1024,768);
  
   //sheet_refresh_rect(sheets);
  //sheet_mouse=sheet_alloc(sheets);
	//u8* sheet_buf4;
   //u8* sheet_buffer=0;
   //sheet_buf4=(u8*)sys_malloc(sheet_mouse->width*sheet_mouse->height);
   //drawmouse(sheet_buffer);
   //sheet_setbuf(sheet_mouse,sheet_buffer);
	//sheet_set_layer(sheets,sheet_mouse,200);
   //sheet_setsheet(sheet_mouse,12,12,100,100);
   // disable_int();
   // init_pci();
	// pci_dev_t* pcid=get_pci_bga();
   // vga_screen_width=1024;
   // vga_screnn_height=768;
   // init_bga(pcid);
   // enable_int();


   sheets=sheets_init();
   set_bkcolor(sheets,rgb_Black);
   gui_mode=1;

   sheet_mouse = sheet_alloc(sheets);
	sheet_setsheet(sheet_mouse, 12, 12, 100, 100);
	u32 *sheet_buf4 = (u32 *)K_PHY2LIN(sys_kmalloc(sheet_mouse->width * sheet_mouse->height*4));
	drawmouse(sheet_buf4);
	sheet_setbuf(sheet_mouse, sheet_buf4);
	sheet_set_layer(sheets,sheet_mouse,255);
   
     
  
   
   //sheet_test();
   //win_test();
   // win_cmd_put_string(mywin,"dsfdf");
   
   
   // for (int i =0; i < memsize; i++)
   // {
   //    *(p++)=0x1;
   // }
   //memset((u8*)K_PHY2LIN(0xA0000),Blue,0x1ffff);
   //cmd_window_write_string(100,100,"hello,os",Red,Blue);
   //memset((u8*)memstart,Red,320*200);
   //drawline(20,20,200,Red);
   //u8* buffer=sys_kmalloc(320*200);
  // draw_rect(buffer,Black,100,50,100,100);
   
  // memcpy((u8 *)memstart, buffer, 320*200);
   // rectangle(100,20,300,100,Red);
   // cmd_window_write_char(100,100,(int)'d',Red,Black);
   // cmd_window_write_string(100,100,"hello123",Red,Blue);
   //cmd_window_draw_mouse(100, 100, Blue, Black);
  // while(1)
   // {
   // //sheets->need_update=TRUE;
  
   // }
}

void putPoint(int x, int y, int Color) /* 画点函数 */
{
   u8 *p;

   p = (u8*)memstart;
   *(x + y * 320 + p) = Color;
}

void drawline(int row, int start, int end, int color)
// draw a line form x to y
{
   char *lstart, *lend;
   lstart = (char *)(memstart + row * 320 + start);
   // lend = (char *)(memstart+row*320+end);

   memset((void *)lstart, color, end - start);
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