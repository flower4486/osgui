#include "type.h"

typedef struct _MY_WINDOW MY_WINDOW;


typedef struct _MY_RECT
{
	int rx;
	int ry;
	int width;
	int height;
	u8	color;
} MY_RECT;

struct _MY_WINDOW
{
	struct sheet* sheet;
	//u8* winbuf;
	
	MY_WINDOW * nxt;
	MY_WINDOW * pre;

	MY_RECT title_rect;
	MY_RECT close_btn_rect;
	MY_RECT cmd_rect;

	int cmd_cursor_x;
	int cmd_cursor_y;
	int cmd_font_width;
	int cmd_font_height;

	u8 cmd_font_color;
};


MY_WINDOW* alloc_window();
void init_window(MY_WINDOW* mywin);
void draw_win_rect(MY_WINDOW* mywin);
void win_test();
void win_cmd_put_char(MY_WINDOW* mywin,u8 ahcar);
void win_cmd_put_string(MY_WINDOW* mywin,char* s);
void drawmouse(u8* buf);
void win_sheet_put_char(MY_WINDOW* mywin,int x,int y,int achar,u8 color,u8 bkcolor);