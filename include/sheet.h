#include "type.h"



#define MAX_SHEETS 256
#define SHEET_USE 1
#define SHEET_NOT_USE 0
struct sheet
{
    
    
    int width,height,x,y,isuse,id;
    u32* buf;
};
struct sheets
{
    u32 *videostart;
    //u32* buffer;
    //int width,height,top;
    
    struct sheet sheets[MAX_SHEETS];
    int need_update;
    //u32* buffer;
    struct sheetnode* sheet0;
};
struct sheetnode
{
    struct sheetnode* nxt;
    struct sheetnode* pre;
    int layer;
    struct sheet* sheet;
};

struct sheets *sheets_init();
struct sheet *sheet_alloc(struct sheets *sheets);
void sheet_setsheet(struct sheet *sheet, int width, int height,int x,int y);
void sheet_free(struct sheet *sheet,struct sheets* sheets);
// void sheet_refresh(struct sheets *sheets);
void sheet_set_layer(struct sheets *sheets, struct sheet *sheet, int layer);
void sheet_test();
void sheet_refresh_rect(struct sheets *sheets);
void sheet_change_output(struct sheets *sheets);
void sheet_setbuf(struct sheet* sheet,u32* buf);
void sheet_slide(struct sheets* sheets,struct sheet* sheet,int mx,int my);
void set_bkcolor(struct sheets* sheets,u32 color);
void sheet_set_top(struct sheet* sheet);