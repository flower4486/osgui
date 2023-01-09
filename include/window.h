#include "type.h"

typedef struct _MY_WINDOW MY_WINDOW;


typedef struct _MY_RECT
{
	int x;
	int y;
	int width;
	int height;
} MY_RECT;

struct _MY_WINDOW
{
	int x;
	int y;
	int width;
	int height;
	u32 * bitmap;
	//MY_WINDOW_CALLBACK mywin_callback;
	void * task;
	u8 need_update;
	MY_RECT update_rect;
	MY_WINDOW * next;
	MY_WINDOW * prev;
	void * kbd_task;
	u8 has_title;
	u8 has_cmd;
	MY_RECT title_rect;
	MY_RECT close_btn_rect;
	MY_RECT cmd_rect;
	int cmd_cursor_x;
	int cmd_cursor_y;
	u32 cmd_font_color;
	u32 cmd_back_color;
	u8 cmd_single_line_out;
};


static int draw_my_window(MY_WINDOW * my_window, 
		u8 * need_update_mouse,MY_RECT * mouse_update_rect);
int create_my_window(MY_WINDOW * my_window);
static u8 detect_intersect_rect(MY_RECT * rect1, MY_RECT * rect2, MY_RECT * d_rect);