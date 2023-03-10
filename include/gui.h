

#include "type.h"



void init_gui();
void do_set_screen(int width,int height);
void putPoint(int x, int y, int Color);                    // put a point
void drawline(int row, int start, int end, int color);     // draw a line
void rectangle(int x1, int y1, int x2, int y2, int Color); /* 画一矩形*/
void vga_write_string(int x, int y, char *s, u32 color, u32 backcolor);
void vga_write_char(int x, int y, int val, u32 color, u32 backColour);
void vga_draw_mouse(int x, int y, u32 color, u32 backColour);
int draw_rect(u8 * buffer, int color, int x, int y, int w, int h);
