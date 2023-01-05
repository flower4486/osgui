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

#define memstart 0xA0000
#define memsize 64000
#define width 320
#define cursor_side 6
#define height 200
void gui();
void putPoint(int x,int y,int Color);//put a point
void drawline(int row,int start,int end,int color);//draw a line
void rectangle(int x1, int y1, int x2, int y2, int Color);  /* 画一矩形*/
void gui(){
vga_write_regs(vga_320x200x256);
 u8 *p;

 p=(u8 *)memstart;
      for(int i=0;i<memsize;i++) *p++=Black;
      *(p+memsize/2)=0x26;
 
    rectangle(100,20,200,100,Red);
 }

 void putPoint(int x, int y, int Color)   /* 画点函数 */
{
   char  *p;

   p = (char*) (0xa0000);
   *(x+y*320+p) = Color;
}

void drawline(int row,int start,int end,int color)
// draw a line form x to y
{
    char *lstart,*lend;
    lstart = (char *)(0xa0000+row*320+start);
    //lend = (char *)(0xa0000+row*320+end);

    memset((void *)lstart,color,end-start);
}
void rectangle(int x1, int y1, int x2, int y2, int Color)  /* 画一矩形*/

{
   for (int i = x1; i < x2; i++)
   {
     for (int j = y1; j < y2; j++)
     {
        putPoint(i,j,Color);
     }
     
   }
   
}
 