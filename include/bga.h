#pragma once
#include "pci.h"
#include "type.h"

#define BGA_SWAP_BUFFERS    0x0101
#define BGA_GET_HEIGHT      0x0102
#define BGA_GET_WIDTH       0x0103
#define BGA_GET_SCALE       0x0104
#define BGA_DISABLE         0x0105
#define BGA_SET_WIDTH       0x0106
#define BGA_SET_HEIGHT      0x0107
#define BGA_SET_UPDATE      0x0108
#define BGA_GET_BUFFER      0x0109

int init_bga(pci_dev_t* dev);
int bga_ioctl(uintptr_t cmd, uintptr_t arg);


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

void bga_set_resolution(u32 width, u32 height);