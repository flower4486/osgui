#include "bga.h"
#include "assert.h"
#include "console.h"
#include "const.h"
#include "protect.h"
#include "proc.h"
#include "global.h"
#include "keyboard.h"
#include "memman.h"
#include "pci.h"
#include "proto.h"
#include "stdio.h"
#include "string.h"
#include "tty.h"
#include "type.h"
#include "x86.h"

u16 vga_screen_width, vga_screnn_height;
u32 vga_screen_line_size, bga_screen_buffer_size;
u32 vga_video_start;
u8  vga_curren_gmem;

static inline void _bga_write_reg(u16 cmd, u16 data) {
    outw(VBE_DISPI_IOPORT_INDEX, cmd);
    outw(VBE_DISPI_IOPORT_DATA, data);
}

static inline u16 _bga_read_reg(u16 cmd) {
    outw(VBE_DISPI_IOPORT_INDEX, cmd);
    return inw(VBE_DISPI_IOPORT_DATA);
}

static void _bga_set_resolution(u16 width, u16 height) {
    #ifndef DEBUG
    _bga_write_reg(VBE_DISPI_INDEX_ENABLE, VBE_DISPI_DISABLED);
    _bga_write_reg(VBE_DISPI_INDEX_XRES, width);
    _bga_write_reg(VBE_DISPI_INDEX_YRES, height);
    _bga_write_reg(VBE_DISPI_INDEX_VIRT_WIDTH, width);
    _bga_write_reg(VBE_DISPI_INDEX_VIRT_HEIGHT, (u16)height * 2);
    _bga_write_reg(VBE_DISPI_INDEX_BPP, 32);
    _bga_write_reg(VBE_DISPI_INDEX_X_OFFSET, 0);
    _bga_write_reg(VBE_DISPI_INDEX_Y_OFFSET, 0);
    _bga_write_reg(VBE_DISPI_INDEX_ENABLE,
                   VBE_DISPI_ENABLED | VBE_DISPI_LFB_ENABLED);
    _bga_write_reg(VBE_DISPI_INDEX_BANK, 0);
    #endif
    vga_screen_line_size = (u32)width * 4;
    vga_curren_gmem=0;

}

void bga_set_resolution(u32 width, u32 height) 
{
    _bga_set_resolution(width, height);
    vga_screen_width = width;
    vga_screnn_height = height;
    bga_screen_buffer_size = vga_screen_line_size * height * 2;

}

int bga_ioctl(uintptr_t cmd, uintptr_t arg)
 {
    u32 y_offset = 0;
    switch (cmd) {
        case BGA_GET_HEIGHT:
            return vga_screnn_height;
        case BGA_GET_WIDTH:
            return vga_screen_width;
        case BGA_GET_SCALE:
            return 1;
        case BGA_GET_BUFFER:
            if (arg == 0)
                return vga_video_start;
            else
                return vga_video_start + bga_screen_buffer_size / 2;
        case BGA_SWAP_BUFFERS:
            y_offset = vga_screnn_height * (arg & 1);
            _bga_write_reg(VBE_DISPI_INDEX_Y_OFFSET, (u16)y_offset);
            return 0;
        case BGA_DISABLE:
            _bga_write_reg(VBE_DISPI_INDEX_ENABLE, VBE_DISPI_DISABLED);
            return 0;
        case BGA_SET_WIDTH:
            vga_screen_width = arg;
            return 0;
        case BGA_SET_HEIGHT:
            vga_screnn_height = arg;
            return 0;
        case BGA_SET_UPDATE:
            bga_set_resolution(vga_screen_width, vga_screnn_height);
            return 0;
        default:
            return -1;
    }
}

int init_bga(pci_dev_t *dev) {
    if (dev->type != DEVICE_DISPLAY) {
        return -1;
    }
    
    vga_video_start = pci_read_bar(dev, 0) & 0xfffffff0;
    u32 err_temp;

    return 0;
}