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
#include "window.h"
#include "sheet.h"

struct sheets* sheets;
// void sheet_test(){
//     disable_int();
//      u8* p=(u8 *)K_PHY2LIN(0xa0000);
//     /// ////////////////////
//     sheets=sheets_init(p,320,200);
//     struct sheet* sheet1=sheet_alloc(sheets);
//     struct sheet* sheet2=sheet_alloc(sheets);
//     struct sheet* sheet3=sheet_alloc(sheets);
//     struct sheet* sheet4_mouse = sheet_alloc(sheets);
    
//     sheet_setsheet(sheet1,100,100,0,0);
//     u8* sheet_buf1=(u8*)sys_malloc(sheet1->height*sheet1->width);
//     memset(sheet_buf1,Blue,sheet1->height*sheet1->width);
//     sheet_setbuf(sheet1,sheet_buf1);

    
//     sheet_setsheet(sheet2,100,100,100,0);
//     u8* sheet_buf2=(u8*)sys_malloc(sheet2->height*sheet2->width);
//     memset(sheet_buf2,Red,sheet2->height*sheet2->width);
//     sheet_setbuf(sheet2,sheet_buf2);


//     sheet_setsheet(sheet3,100,100,200,0);
//     u8* sheet_buf3=(u8*)sys_malloc(sheet3->width*sheet3->height);
//     memset(sheet_buf3,Yellow,sheet3->width*sheet3->height);
//     sheet_setbuf(sheet3,sheet_buf3);

    
//     sheet_setsheet(sheet4_mouse,12,12,0,0);
//     u8* sheet_buf4=(u8*)sys_malloc(sheet4_mouse->width*sheet4_mouse->height);
//     drawmouse(0,0);
//     sheet_setbuf(sheet4_mouse,sheet_buf4);

//     sheet_set_layer(sheets,sheet1,3);
//     sheet_set_layer(sheets,sheet2,5);
//     sheet_set_layer(sheets,sheet3,8);
//     sheet_set_layer(sheets,sheet4_mouse,256);
//     sheet_refresh_rect(sheets);

//      sheet_slide(sheets,sheet3,150,0);
//     // sheet_slide(sheets,sheet1,200,0);
//     enable_int();
// }

void set_bkcolor(struct sheets* sheets,int color)
{
    disable_int();
    struct sheet* bksheet=sheet_alloc(sheets);
    sheet_setsheet(bksheet,320,200,0,0);
    u8* bksheet_buf=(u8*)sys_malloc(320*200);
    memset(bksheet_buf,color,320*200);
    sheet_setbuf(bksheet,bksheet_buf);
    sheet_set_layer(sheets,bksheet,0);
    enable_int();
}

struct sheets *sheets_init(u8 *memstart, int width, int height)
{
    struct sheets *sheets;
    int i;
    sheets = (struct sheets *)sys_kmalloc(sizeof(struct sheets));
    if (sheets == 0)
    {
        return NULL;
    }
    sheets->videostart = memstart;
    sheets->width = width;
    sheets->height = height;
    sheets->top = -1;
    for (i = 0; i < MAX_SHEETS; i++)
    {
        sheets->sheets[i].isuse = SHEET_NOT_USE;
        sheets->sheets[i].id = i;
    }
    sheets->sheet0=(struct sheetnode*)sys_kmalloc(sizeof(struct sheetnode));
    return sheets;
}

struct sheet *sheet_alloc(struct sheets *sheets)
{
    struct sheet *sheet;
    int i;
    for (int i = 0; i < MAX_SHEETS; i++)
    {
        if (sheets->sheets[i].isuse == 0)
        {
            sheet = &sheets->sheets[i];
            sheet->isuse = SHEET_USE;
            return sheet;
        }
    }
    return 0;
};

void sheet_setsheet(struct sheet *sheet,int width, int height,int x,int y)
{

    sheet->width = width;
    sheet->height = height;
    sheet->x=x;
    sheet->y=y;
    return;
}

void sheet_setbuf(struct sheet* sheet,u8* buf)
{
    sheet->buf = buf;
    return;
}

void sheet_set_layer(struct sheets *sheets, struct sheet *sheet, int layer)
{

    if (layer >= MAX_SHEETS)
    {
        layer = MAX_SHEETS-1;
    }
    if (layer < -1)
    {
        layer = -1;
    }

    struct sheetnode *sheet_head=sheets->sheet0;
    struct sheetnode *sheet_cur= sheets->sheet0;
    struct sheetnode *tmpnode;

 
    while (sheet_cur->nxt!=NULL&&sheet_cur->nxt->sheet->id != sheet->id)
    {
        sheet_cur= sheet_cur->nxt;
        if (sheet_cur->nxt==NULL)
        {
            break;
        }
    }
    
   
    struct sheetnode *newnode;
    if (sheet_cur->nxt!= NULL)
    {
        sheet_cur->nxt->pre=sheet_cur->pre;
        newnode = sheet_cur->nxt;
        sheet_cur= sheet_cur->nxt->nxt;
    }else{
        newnode=(struct sheetnode*)sys_kmalloc(sizeof(struct sheetnode));
        newnode->sheet=sheet;
    }

    newnode->layer=layer;


    sheet_cur= sheet_head;
    while (sheet_cur->nxt!=NULL&&sheet_cur->nxt->layer < layer)
    {
        sheet_cur= sheet_cur->nxt;
    }

    if (sheet_cur->nxt==NULL)
    {
        sheet_cur->nxt=newnode;
        newnode->pre=sheet_cur;
        newnode->nxt=NULL;
    }else if (sheet_cur->nxt->layer == layer)
    {
        sheet_set_layer(sheets, sheet, layer + 1);
        return;
    }else{
        tmpnode=sheet_cur->nxt;
        sheet_cur->nxt=newnode;
        newnode->pre=sheet_cur;
        newnode->nxt=tmpnode;
        tmpnode->pre=newnode;
    }
    return;
}

// void sheet_refresh(struct sheets *sheets)
// {

//     struct sheetnode *sheet_cur= sheets->sheet0->nxt;

//     while (sheet_cur!= NULL)
//     {
//         memcpy(sheets->videostart,sheet_cur->sheet->buf,sheet_cur->sheet->height*sheet_cur->sheet->width);
//         sheet_cur=sheet_cur->nxt;
//     }
// }

void sheet_refresh_rect(struct sheets *sheets)
{
    struct sheetnode* sheet_cur=sheets->sheet0->nxt;

    while (sheet_cur!=NULL)
    {
        int offset=sheet_cur->sheet->x+sheet_cur->sheet->y*320;
        for (int i = 0; i < sheet_cur->sheet->height; i++)
        {
           memcpy(sheets->videostart+offset,(sheet_cur->sheet->buf+sheet_cur->sheet->width*i),sheet_cur->sheet->width);
           offset+=320;
        }
        sheet_cur=sheet_cur->nxt;
    }    
}

void sheet_free(struct sheet *sheet,struct sheets* sheets)
{
    if (sheet->isuse)
    {
    struct sheetnode *sheet_cur= sheets->sheet0;
    struct sheetnode *tmpnode;
    while (sheet_cur->nxt->sheet->id != sheet->id&&sheet_cur->nxt!=NULL)
        sheet_cur= sheet_cur->nxt;
    if (sheet_cur->nxt!= NULL)
    {
        tmpnode = sheet_cur->nxt;
        sheet_cur= sheet_cur->nxt->nxt;
        sys_free(tmpnode);
    }
    sheet->isuse=SHEET_NOT_USE;
    }
    
}

void sheet_slide(struct sheets* sheets,struct sheet* sheet,int newx,int newy)
{
    sheet->x=newx;
    sheet->y=newy;
    if (sheet->isuse)
    {
        sheet_refresh_rect(sheets);
    }
    return;
    
}