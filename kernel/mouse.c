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
#include "keymap.h"
#include "x86.h"
#include "stdio.h"
#include "sheet.h"
#include "window.h"
#include "mouse.h"
static MOUSE_INPUT mouse_in;
static int mouse_init;
struct sheet *sheet_mouse;
struct sheet* mouse_bind_sheet;
void mouse_move()
{
	sheet_setsheet(sheet_mouse, 12, 12, sheet_mouse->x, sheet_mouse->y);
}
void mouse_handler(int irq)
{
	u8 scan_code = inb(0x60);
	TTY *p_tty = 0;
	if (!mouse_init)
	{
		mouse_init = 1;
		return;
	}
	
	// 等待鼠标的三个字节消息全部来到
	mouse_in.buf[mouse_in.count] = scan_code;
	mouse_in.count++;
	if (mouse_in.count == 3)
	{
		//kprintf("%d ",mouse_in.buf[0]);
		p_tty->mouse_left_button = mouse_in.buf[0] & 0x01;
		p_tty->mouse_mid_button = mouse_in.buf[0] & 0x4;
		u8 dx_sign = mouse_in.buf[0]&0x10;
		u8 dy_sign = mouse_in.buf[0]&0x20;
		//kprintf("%d %d ",x,-y);
		int dx,dy;
		if(dy_sign==0x20){
			//down
			dy=1;
		}else{
			dy=-1;
		}
		if(dx_sign==0x10){
			dx=-1;
		}else{
			dx=1;
		}

		<<<<<<< master
		//sheet_mouse->x+=6*dx;
		//sheet_mouse->y+=5*dy;
		if (gui_mode==1)
		{
		 //mouse_move();
		 sheet_slide(sheets,sheet_mouse,4*dx,3*dy);
		 if (mouse_bind_sheet!=NULL)
		 {
			sheet_slide(sheets,mouse_bind_sheet,4*dx,3*dy);
		 }
		 
		}
		
// =======2023-01-28 
// 		sheet_mouse->x+=dx;
// 		sheet_mouse->y+=dy;

// >>>>>>> master
		mouse_in.count = 0;
	}
	
}

void init_mouse()
{
	mouse_in.count = 0;

	put_irq_handler(MOUSE_IRQ, mouse_handler);
	enable_irq(MOUSE_IRQ);
}
