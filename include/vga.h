/*vga.h the vga header*/

#ifndef _VGA_H_
#define _VGA_H_

#include "type.h"
#define VGA_CHAR_HEIGHT 12
#define VGA_CHAR_WIDTH 8

#define VGA_MODE_80X25_TEXT 0
#define VGA_MODE_320X200X256 1
#define VGA_MODE_640X480X16 2
#define VGA_MODE_VBE_1024X768X32 3

#define VGA_AC_INDEX 0x3C0
#define VGA_AC_WRITE 0x3C0
#define VGA_AC_READ 0x3C1
#define VGA_MISC_WRITE 0x3C2
#define VGA_SEQ_INDEX 0x3C4
#define VGA_SEQ_DATA 0x3C5
#define VGA_DAC_READ_INDEX 0x3C7
#define VGA_DAC_WRITE_INDEX 0x3C8
#define VGA_DAC_DATA 0x3C9
#define VGA_MISC_READ 0x3CC
#define VGA_GC_INDEX 0x3CE
#define VGA_GC_DATA 0x3CF
/* COLOR emulation MONO emulation */
#define VGA_CRTC_INDEX 0x3D4 /* 0x3B4 */
#define VGA_CRTC_DATA 0x3D5 /* 0x3B5 */
#define VGA_INSTAT_READ 0x3DA

#define VGA_NUM_SEQ_REGS 5
#define VGA_NUM_CRTC_REGS 25
#define VGA_NUM_GC_REGS 9
#define VGA_NUM_AC_REGS 21
#define VGA_NUM_REGS (1 + VGA_NUM_SEQ_REGS + VGA_NUM_CRTC_REGS + \
				VGA_NUM_GC_REGS + VGA_NUM_AC_REGS)
void vga_write_regs(u8 *regs);
int vga_update_screen(u8 * buffer, u32 buffer_size);

extern u8 vga_320x200x256[];

#endif // _VGA_H_

