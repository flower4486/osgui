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
static MOUSE_INPUT mouse_in;
static int mouse_init;
struct sheet *sheet_mouse;
void mouse_test()
{
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
		u8 x = mouse_in.buf[1];
		u8 y = mouse_in.buf[2];
		kprintf("%d %d ",x,-y);

		if (gui_mode == 1)
		{
			sheet_setsheet(sheet_mouse, 12, 12, x+sheet_mouse->x, sheet_mouse->y-y);
		}
		mouse_in.count = 0;
	}

	
}

void init_mouse()
{
	mouse_in.count = 0;

	put_irq_handler(MOUSE_IRQ, mouse_handler);
	enable_irq(MOUSE_IRQ);
}