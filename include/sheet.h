#include "type.h"



#define MAX_SHEETS 256
#define SHEET_USE 1
#define SHEET_NOT_USE 0
struct sheet
{
    // u8 *buf;
    u8 buffer[];
    int width,height,x,y,isuse,id;
    
};
struct sheets
{
    u8 *videostart;
    int width,height,top;
    struct sheetnode* sheet0;
    struct sheet sheets[MAX_SHEETS];
};
struct sheetnode
{
    struct sheetnode* nxt;
    struct sheetnode* pre;
    int layer;
    struct sheet* sheet;
};

struct sheets *sheets_init(u8 *memstart, int width, int height);
struct sheet *sheet_alloc(struct sheets *sheets);
void sheet_setsheet(struct sheet *sheet, int width, int height,int x,int y);
void sheet_free(struct sheet *sheet,struct sheets* sheets);
// void sheet_refresh(struct sheets *sheets);
void sheet_set_layer(struct sheets *sheets, struct sheet *sheet, int layer);
void sheet_test();
void sheet_refresh_rect(struct sheets *sheets);
void sheet_setbuf(struct sheet* sheet,u8* buf);
void sheet_slide(struct sheets* sheets,struct sheet* sheet,int newx,int newy);
void set_bkcolor(struct sheets* sheets,int color);