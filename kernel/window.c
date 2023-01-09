#include "window.h"
#include "type.h"
#include "const.h"
#include "string.h"
#include "protect.h"
#include "tty.h"
#include "proc.h"

#include "proto.h"
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

int my_mouse_x = 0;
int my_mouse_y = 0;
int my_mouse_w = 0;
int my_mouse_h = 0;
u8 my_mouse_left_press = FALSE;
int my_mouse_left_press_x = 0;
int my_mouse_left_press_y = 0;
MY_RECT my_drag_rect = {0};
MY_WINDOW * my_drag_win = NULL;
u8 my_mouse_isInit = FALSE;

static int draw_my_window(MY_WINDOW * my_window, 
		u8 * need_update_mouse,MY_RECT * mouse_update_rect);
int create_my_window(MY_WINDOW * my_window);

int create_my_window(MY_WINDOW * my_window)
{
	MY_WINDOW * mywin = (MY_WINDOW *)sys_kmalloc(sizeof(MY_WINDOW));
	memset((u8 *)mywin, 0, sizeof(MY_WINDOW));
	int pix_byte = lfb_resolution_b / 8;
	mywin->x = my_window->x;
	mywin->y = my_window->y;
	mywin->width = my_window->width;
	mywin->height = my_window->height;
	mywin->bitmap = (u32 *)sys_kmalloc(my_window->width * my_window->height * pix_byte);
	//mywin->mywin_callback = my_window->mywin_callback;
	mywin->task = my_window->task;
	mywin->kbd_task = my_window->task;
	//((ZLOX_TASK *)mywin->task)->mywin = mywin;
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
			if(tmpwin->next == NULL)
			{
				tmpwin->next = mywin;
				mywin->prev = tmpwin;
				break;
			}
			else
			{
				tmpwin = tmpwin->next;
			}
		}
		mywin_list_end = mywin;
	}
	//ZLOX_TASK_MSG msg = {0};
	//msg.type = ZLOX_MT_CREATE_MY_WINDOW;
	//zlox_send_tskmsg((ZLOX_TASK *)mywin->task,&msg);
	return (int)mywin;
}

static int draw_my_window(MY_WINDOW * my_window, 
		u8 * need_update_mouse,MY_RECT * mouse_update_rect)
{
	MY_WINDOW * mywin = my_window;
	u32 * vid_mem = (u32 *)lfb_vid_memory;
	int x, y , j, w, h, src_x, src_y;
	int pix_byte = lfb_resolution_b / 8;
	if(mywin->need_update)
	{
		x = mywin->x + mywin->update_rect.x;
		y = mywin->y + mywin->update_rect.y;
		src_x = mywin->update_rect.x;
		src_y = mywin->update_rect.y;
		w = mywin->update_rect.width;
		h = mywin->update_rect.height;
	}
	else
	{
		x = mywin->x;
		y = mywin->y;
		src_x = 0;
		src_y = 0;
		w = mywin->width;
		h = mywin->height;
	}
	MY_RECT tmp_rect;
	MY_RECT lfb_tmp_rect;
	MY_RECT intersect_rect;
	tmp_rect.x = x;
	tmp_rect.y = y;
	tmp_rect.width = w;
	tmp_rect.height = h;
	lfb_tmp_rect.x = 0;
	lfb_tmp_rect.y = 0;
	lfb_tmp_rect.width = lfb_resolution_x;
	lfb_tmp_rect.height = lfb_resolution_y;
	if(detect_intersect_rect(&tmp_rect, &lfb_tmp_rect, &intersect_rect) == FALSE)
	{
		if(need_update_mouse != NULL)
			(*need_update_mouse) = FALSE;
		return -1;
	}
	x = intersect_rect.x;
	y = intersect_rect.y;
	src_x = intersect_rect.x - mywin->x;
	src_y = intersect_rect.y - mywin->y;
	w= intersect_rect.width;
	h = intersect_rect.height;
	for(j = 0; j < h ; y++, src_y++, j++)
	{
		memcpy((u8 *)&vid_mem[x + y * lfb_resolution_x], 
				(u8 *)&mywin->bitmap[src_x + src_y * mywin->width], 
				w * pix_byte);
	}
	if(need_update_mouse != NULL && mouse_update_rect != NULL)
	{
		tmp_rect.x = my_mouse_x;
		tmp_rect.y = my_mouse_y;
		tmp_rect.width = my_mouse_w;
		tmp_rect.height = my_mouse_h;
		(*need_update_mouse) = detect_intersect_rect(&tmp_rect, &intersect_rect, mouse_update_rect);
	}
	return 0;
}

static u8 detect_intersect_rect(MY_RECT * rect1, MY_RECT * rect2, MY_RECT * d_rect)
{
	int r1_x = rect1->x + rect1->width - 1;
	int r1_y = rect1->y + rect1->height - 1;
	int r2_x = rect2->x + rect2->width - 1;
	int r2_y = rect2->y + rect2->height - 1;
	int d_x = 0;
	int d_y = 0;

	if(rect1->x >= rect2->x && rect1->x <= r2_x)
		d_rect->x = rect1->x;
	else if(rect2->x >= rect1->x && rect2->x <= r1_x)
		d_rect->x = rect2->x;
	else
		return FALSE;

	if(rect1->y >= rect2->y && rect1->y <= r2_y)
		d_rect->y = rect1->y;
	else if(rect2->y >= rect1->y && rect2->y <= r1_y)
		d_rect->y = rect2->y;
	else
		return FALSE;

	if(r1_x >= rect2->x && r1_x <= r2_x)
		d_x = r1_x;
	else if(r2_x >= rect1->x && r2_x <= r1_x)
		d_x = r2_x;
	else
		return FALSE;

	if(r1_y >= rect2->y && r1_y <= r2_y)
		d_y = r1_y;
	else if(r2_y >= rect1->y && r2_y <= r1_y)
		d_y = r2_y;
	else
		return FALSE;

	d_rect->width = d_x - d_rect->x + 1;
	d_rect->height = d_y - d_rect->y + 1;
	return TRUE;
}