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
u32* sheets_bitmap;
struct sheets *sheets;
void sheet_test()
{
    disable_int();

    /// ////////////////////
    struct sheet *sheet1 = sheet_alloc(sheets);
    struct sheet *sheet2 = sheet_alloc(sheets);
    struct sheet *sheet3 = sheet_alloc(sheets);


    sheet_setsheet(sheet1, 100, 100, 0, 0);
    u32 *sheet_buf1 = (u32 *)K_PHY2LIN(sys_kmalloc(100 * 100 * 4));
    fastset((void*)sheet_buf1, rgb_Blue, 100 * 100);
    

    sheet_setbuf(sheet1,(u32*) sheet_buf1);

    // sheet_setsheet(sheet2, 100, 100, 100, 0);
    // u32 *sheet_buf2 = (u32 *)sys_malloc(sheet2->width * sheet2->height * 4);
    // fastset(sheet_buf2, rgb_Red, sheet2->width * sheet2->height);
    // sheet_setbuf(sheet2, sheet_buf2);

    // sheet_setsheet(sheet3, 100, 100, 200, 0);
    // u32 *sheet_buf3 = (u32 *)sys_malloc(sheet3->width * sheet3->height * 4);
    // fastset(sheet_buf3, rgb_Green, sheet3->width * sheet3->height);
    // sheet_setbuf(sheet3, sheet_buf3);

    sheet_set_layer(sheets, sheet1, 3);
    // sheet_set_layer(sheets, sheet2, 5);
    // sheet_set_layer(sheets, sheet3, 8);

    sheet_refresh_rect(sheets);

    //   sheet_slide(sheets,sheet3,150,0);
    // sheet_slide(sheets,sheet1,200,0);
    enable_int();
}

void set_bkcolor(struct sheets *sheets, u32 color)
{
    disable_int();
    struct sheet *bksheet = sheet_alloc(sheets);
    sheet_setsheet(bksheet, vga_screen_width, vga_screnn_height, 0, 0);

    u32 *bk_buffer = (u32 *)K_PHY2LIN(sys_kmalloc(vga_screen_width * vga_screnn_height * 4));
    sheet_setbuf(bksheet, bk_buffer);
    sheet_set_layer(sheets, bksheet, 0);

    fastset((void*)bksheet->buf,color,bksheet->width*bksheet->height);
    sheets->need_update=TRUE;
   // sheet_refresh_rect(sheets);
    enable_int();
}

struct sheets *sheets_init()
{
    struct sheets *sheets;
    int i;
    sheets = (struct sheets *)K_PHY2LIN(sys_kmalloc(sizeof(struct sheets)));
    sheets_bitmap=(u32*)K_PHY2LIN(sys_kmalloc(vga_screen_width*vga_screnn_height*4));
    if (sheets == 0)
    {
        return NULL;
    }
    sheets->videostart = (u32 *)vga_video_start;
    for (i = 0; i < MAX_SHEETS; i++)
    {
        sheets->sheets[i].isuse = SHEET_NOT_USE;
        sheets->sheets[i].id = i;
    }
    sheets->sheet0 = (struct sheetnode *)K_PHY2LIN(sys_kmalloc(sizeof(struct sheetnode)));
    memset((u8*)sheets->sheet0,0,sizeof(struct sheetnode));
    sheets->sheet0->nxt=sheets->sheet0->pre=0;
    //sheets->buffer=(u32*)sheets_bitmap;
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

void sheet_setsheet(struct sheet *sheet, int width, int height, int x, int y)
{
    
    sheet->width = width;
    sheet->height = height;
    if (x>=vga_screen_width-sheet->width)
    {
        x=vga_screen_width-sheet->width;
    }else if (x<=0)
    {
        x=0;
    }
    
    if (y>=vga_screnn_height-sheet->height)
    {
        y=vga_screnn_height-sheet->height;
    }else if (y<=0)
    {
        y=0;
    }    
    sheet->x = x;
    sheet->y = y;
    sheets->need_update=TRUE;
    return;
}

void sheet_setbuf(struct sheet *sheet, u32 *buf)
{
    sheet->buf = buf;
    return;
}

void sheet_set_layer(struct sheets *sheets, struct sheet *sheet, int layer)
{

    if (layer >= MAX_SHEETS)
    {
        layer = MAX_SHEETS - 1;
    }
    if (layer < -1)
    {
        layer = -1;
    }

    struct sheetnode *sheet_head = sheets->sheet0;
    struct sheetnode *sheet_cur = sheets->sheet0;
    struct sheetnode *tmpnode;

    while (sheet_cur->nxt != NULL && sheet_cur->nxt->sheet->id != sheet->id)
    {
        sheet_cur = sheet_cur->nxt;
        if (sheet_cur->nxt == NULL)
        {
            break;
        }
    }

    struct sheetnode *newnode;
    if (sheet_cur->nxt != NULL)
    {
        sheet_cur->nxt->pre = sheet_cur->pre;
        newnode = sheet_cur->nxt;
        sheet_cur = sheet_cur->nxt->nxt;
    }
    else
    {
        newnode = (struct sheetnode *)K_PHY2LIN(sys_kmalloc(sizeof(struct sheetnode)));
        newnode->sheet = sheet;
    }

    newnode->layer = layer;

    sheet_cur = sheet_head;
    while (sheet_cur->nxt != NULL && sheet_cur->nxt->layer < layer)
    {
        sheet_cur = sheet_cur->nxt;
    }

    if (sheet_cur->nxt == NULL)
    {
        sheet_cur->nxt = newnode;
        newnode->pre = sheet_cur;
        newnode->nxt = NULL;
    }
    else if (sheet_cur->nxt->layer == layer)
    {
        sheet_set_layer(sheets, sheet, layer + 1);
        return;
    }
    else
    {
        tmpnode = sheet_cur->nxt;
        sheet_cur->nxt = newnode;
        newnode->pre = sheet_cur;
        newnode->nxt = tmpnode;
        tmpnode->pre = newnode;
    }

    sheets->need_update=TRUE;
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
    struct sheetnode *sheet_cur = sheets->sheet0->nxt;
    // fastset(sheets_bitmap,rgb_Blue,1024*768);
    if (sheets->need_update)
    {
        while (sheet_cur != NULL)
        {
            //fastset(sheet_cur->sheet->buf,rgb_Yellow,vga_screen_width*vga_screnn_height); 
            int offset = sheet_cur->sheet->x + sheet_cur->sheet->y * vga_screen_width;
            for (int i = 0; i < sheet_cur->sheet->height; i++)
            {               
                // for (int j = 0; j < sheet_cur->sheet->width; j++)
                // {
                //     //*(u32*)(sheets_bitmap+j+sheet_cur->sheet->x+(i+sheet_cur->sheet->y)*vga_screen_width)=//*(sheet_cur->sheet->buf+j+i*sheet_cur->sheet->width);
                //     sheets_bitmap[j+sheet_cur->sheet->x+(i+sheet_cur->sheet->y)*vga_screen_width]=
                //     rgb_Yellow;
                //     //sheet_cur->sheet->buf[j+i*sheet_cur->sheet->width];
                // }
                
               // fastcpy(sheets->videostart + offset, (sheet_cur->sheet->buf + sheet_cur->sheet->width * i), sheet_cur->sheet->width);
                fastcpy((sheets_bitmap+ offset), (sheet_cur->sheet->buf + sheet_cur->sheet->width * i), sheet_cur->sheet->width);
               // fastcpy((sheets->videostart + offset),sheets->buffer+offset,sheet_cur->sheet->width);
               //fastcpy((sheets_bitmap+ offset), (sheet_cur->sheet->buf + vga_screnn_height * i), vga_screnn_height); 
                          
                offset += vga_screen_width;
            }
            
            sheet_cur = sheet_cur->nxt;
        }
        //fastset(sheets_bitmap,rgb_Yellow,vga_screen_width*vga_screnn_height); 
        sheet_write_graphic_mem();
        sheets->need_update = FALSE;
    }
}

void sheet_write_graphic_mem()
{
    //fastset(sheets_bitmap,rgb_Yellow,vga_screen_width*vga_screnn_height);
    // int offset=0;
     for (int i = 0; i < vga_screnn_height*vga_screen_width; i++)
     {
        //fastcpy(sheets->videostart,sheets_bitmap,vga_screen_width*vga_screnn_height);
        // offset+=vga_screen_width*10;
        *(u32*)(sheets->videostart+i)=
        *((u32*)(sheets_bitmap+i));
        // sheets->videostart[i]=
        //rgb_Blue;
        //*(sheets_bitmap+i);
     }    
}
void sheet_change_output(struct sheets *sheets)
{
    
}
void sheet_free(struct sheet *sheet, struct sheets *sheets)
{
    if (sheet->isuse)
    {
        struct sheetnode *sheet_cur = sheets->sheet0;
        struct sheetnode *tmpnode;
        while (sheet_cur->nxt->sheet->id != sheet->id && sheet_cur->nxt != NULL)
            sheet_cur = sheet_cur->nxt;
        if (sheet_cur->nxt != NULL)
        {
            tmpnode = sheet_cur->nxt;
            sheet_cur = sheet_cur->nxt->nxt;
            sys_free(tmpnode);
        }
        sheet->isuse = SHEET_NOT_USE;
        sheets->need_update=TRUE;
    }
}

void sheet_slide(struct sheets *sheets, struct sheet *sheet, int mx, int my)
{
    sheet->x += mx;
    sheet->y += my;
    if (sheet->x>=vga_screen_width-sheet->width)
    {
        sheet->x=vga_screen_width-sheet->width;
    }else if (sheet->x<=0)
    {
        sheet->x=0;
    }
    
    if (sheet->y>=vga_screnn_height-sheet->height)
    {
       sheet->y=vga_screnn_height-sheet->height;
    }else if (sheet->y<=0)
    {
        sheet->y=0;
    }    
    if (sheet->isuse)
    {
        sheets->need_update=TRUE;
    }
    return;
}

