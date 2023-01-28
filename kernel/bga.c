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

static inline void _bga_write_reg(u16 cmd, u16 data) {
    outw(VBE_DISPI_IOPORT_INDEX, cmd);
    outw(VBE_DISPI_IOPORT_DATA, data);
}

static inline u16 _bga_read_reg(u16 cmd) {
    outw(VBE_DISPI_IOPORT_INDEX, cmd);
    return inw(VBE_DISPI_IOPORT_DATA);
}

static void _bga_set_resolution(u16 width, u16 height) {
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

    vga_screen_line_size = (u32)width * 4;
}

static void bga_set_resolution(u32 width, u32 height) {
    _bga_set_resolution(width, height);
    vga_screen_width = width;
    vga_screnn_height = height;
    bga_screen_buffer_size = vga_screen_line_size * height * 2;
}

int bga_ioctl(uintptr_t cmd, uintptr_t arg) {
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
    //u32 bga_buf_laddr = K_PHY2LIN(vga_video_start);
    //bga_set_resolution(1024, 768);
    bga_set_resolution(vga_screen_width,vga_screnn_height);
    //kprintf("bga buf paddr=%p; buf size= %d KB\n", vga_video_start,
    //        bga_screen_buffer_size / 1024);
    //kprintf("bga mapped in kernel cr3=%p\n", read_cr3());
    for (int k = 0; k < bga_screen_buffer_size; k += 4096) {
        lin_mapping_phy(vga_video_start + k, 
                        vga_video_start + k,
                        p_proc_current->task.pid,
                        //0,
                         PG_P | PG_USU | PG_RWW, PG_P | PG_USU | PG_RWW);
    }
    //u32 *test = (u32 *)vga_video_start;
    //u32 *test2 = (u32 *)(vga_video_start + bga_screen_buffer_size / 2);
    // for (int i = 0; i <= 1024 * 20; ++i) 
    // {
    // test[i] = 0x00ff12;
    // }
    
    // for (int i = 1024 * 20; i <= 1024 * 768; ++i)
    // {
    // test[i] = 0x0000ff;
    // }
    //  while (1) 
    //  {
    //     bga_ioctl(BGA_SWAP_BUFFERS, 1);
    //     for (volatile int i = 0; i < 0x03ffffff; ++i)
    //         ;
    //     bga_ioctl(BGA_SWAP_BUFFERS, 0);
    //     for (volatile int i = 0; i < 0x03ffffff; ++i)
    //         ;
    // }
    return 0;
}