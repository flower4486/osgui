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
#include "vga.h"



#define VBE_DISPI_IOPORT_INDEX 0x01CE
#define VBE_DISPI_IOPORT_DATA 0x01CF

#define VBE_DISPI_INDEX_ID 0x0
#define VBE_DISPI_INDEX_XRES 0x1
#define VBE_DISPI_INDEX_YRES 0x2
#define VBE_DISPI_INDEX_BPP 0x3
#define VBE_DISPI_INDEX_ENABLE 0x4
#define VBE_DISPI_INDEX_BANK 0x5
#define VBE_DISPI_INDEX_VIRT_WIDTH 0x6
#define VBE_DISPI_INDEX_VIRT_HEIGHT 0x7
#define VBE_DISPI_INDEX_X_OFFSET 0x8
#define VBE_DISPI_INDEX_Y_OFFSET 0x9

#define VBE_DISPI_DISABLED 0x00
#define VBE_DISPI_ENABLED 0x01
#define VBE_DISPI_LFB_ENABLED 0x40

static u16 bga_screen_width, bga_screen_height;
static u32 bga_screen_line_size, bga_screen_buffer_size;
static u32 bga_buf_paddr;

// static int _bga_swap_page_mode(struct memzone* zone, uintptr_t vaddr)
// {
//     return SWAP_NOT_ALLOWED;
// }

// static vm_ops_t mmap_file_vm_ops = {
//     .load_page_content = NULL,
//     .restore_swapped_page = NULL,
//     .swap_page_mode = _bga_swap_page_mode,
// };

static inline void _bga_write_reg(u16 cmd, u16 data)
{
    outw(VBE_DISPI_IOPORT_INDEX, cmd);
    outw(VBE_DISPI_IOPORT_DATA, data);
    
}

static inline u16 _bga_read_reg(u16 cmd)
{
    outw(VBE_DISPI_IOPORT_INDEX, cmd);
    return inw(VBE_DISPI_IOPORT_DATA);
}

static void _bga_set_resolution(u16 width, u16 height)
{
    _bga_write_reg(VBE_DISPI_INDEX_ENABLE, VBE_DISPI_DISABLED);
    _bga_write_reg(VBE_DISPI_INDEX_XRES, width);
    _bga_write_reg(VBE_DISPI_INDEX_YRES, height);
    _bga_write_reg(VBE_DISPI_INDEX_VIRT_WIDTH, width);
    _bga_write_reg(VBE_DISPI_INDEX_VIRT_HEIGHT, (u16)height * 2);
    _bga_write_reg(VBE_DISPI_INDEX_BPP, 32);
    _bga_write_reg(VBE_DISPI_INDEX_X_OFFSET, 0);
    _bga_write_reg(VBE_DISPI_INDEX_Y_OFFSET, 0);
    _bga_write_reg(VBE_DISPI_INDEX_ENABLE, VBE_DISPI_ENABLED | VBE_DISPI_LFB_ENABLED);
    _bga_write_reg(VBE_DISPI_INDEX_BANK, 0);

    bga_screen_line_size = (u32)width * 4;
}

void bga_set_resolution(u32 width, u32 height)
{
    _bga_set_resolution(width, height);
    bga_screen_width = width;
    bga_screen_height = height;
    bga_screen_buffer_size = bga_screen_line_size * height * 2;
}