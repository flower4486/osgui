#include "type.h"
#include "assert.h"
#include "console.h"
#include "const.h"
#include "keyboard.h"
#include "protect.h"
#include "tty.h"
#include "memman.h"
#include "proc.h"

#include "proto.h"
#include "stdio.h"
#include "string.h"
#include "x86.h"
#include "global.h"

#include "pci.h"


#define DEBUG_PCI

pci_dev_t pci_devs[NR_PCI_DEV];

static u32 _pci_do_read_bar(u8 bus, u8 device, u8 function,
                                 u8 bar_id) {
    u32 header_type = pci_read(bus, device, function, 0x0e) & 0x7f;
    u8 max_bars = 6 - (header_type * 4);
    if (bar_id >= max_bars) return 0;
    u32 bar_val = pci_read(bus, device, function, 0x10 + 4 * bar_id);
    return bar_val;
}

static bar_t _pci_get_bar(u8 bus, u8 device, u8 function,
                          u8 bar_id) {
    bar_t result;

    u32 bar_val = _pci_do_read_bar(bus, device, function, bar_id);
    result.type = (bar_val & 0x1) ? INPUT_OUTPUT : MEMORY_MAPPED;

    if (result.type == MEMORY_MAPPED) {
    } else {
        result.address = (u32)((bar_val >> 2) << 2);
        result.prefetchable = 0;
    }
    return result;
}

u32 pci_read(u16 bus, u16 device, u16 function,
                  u32 offset) {
    u32 id = (0x1 << 31) | ((bus & 0xFF) << 16) | ((device & 0x1F) << 11) |
                  ((function & 0x07) << 8) | (offset & 0xFC);
    outl(0xCF8, id);
    u32 tmp = (u32)(inl(0xCFC) >> (8 * (offset % 4)));
    return tmp;
}

void pci_write(u8 bus, u8 device, u8 function, u8 offset,
               u32 data) {
    u32 bus32 = bus;
    u32 device32 = device;
    u16 function16 = function;
    u32 address = (1 << 31) | (bus32 << 16) | (device32 << 11) |
                       (function16 << 8) | (offset & 0xFC);
    outl(0xCF8, address);
    outl(0xCFC, data);
}

static int dev_type_by_class(pci_dev_t* dd) {
    switch (dd->class_id) {
        case 0x1:
            switch (dd->subclass_id) {
                case 0x1:
                case 0x3:
                case 0x4:
                    return DEVICE_BUS_CONTROLLER;
                default:
                    return DEVICE_STORAGE;
            }
        case 0x2:
            return DEVICE_NETWORK;
        case 0x3:
            return DEVICE_DISPLAY;
        case 0x6:
            return DEVICE_BRIDGE;
        default:
#ifdef DEBUG_PCI
            kprintf("PCI: DEVICE_UNKNOWN: Class %d subclass %d", dd->class_id,
                    dd->subclass_id);
#endif
            return DEVICE_UNKNOWN;
    }
}

static char pci_has_device_functions(u8 bus, u8 device) {
    return pci_read(bus, device, 0, 0x0e) & (1 << 7);
}

int pci_find_devices() {
#ifdef DEBUG_PCI
    kprintf("PCI: scanning\n");
#endif
    u8 bus, device, function;
	int count = 0;
    for (bus = 0; bus < 8; bus++) {
        for (device = 0; device < 32; device++) {
            u8 functions_count =
                pci_has_device_functions(bus, device) == 0 ? 8 : 1;
            for (function = 0; function < functions_count; function++) {
                pci_dev_t dev =
                    pci_get_device_desriptor(bus, device, function);
                if (dev.vendor_id == 0x0000 || dev.vendor_id == 0xffff) {
                    continue;
                }

                for (u8 bar_id = 0; bar_id < 6; bar_id++) {
                    bar_t bar = _pci_get_bar(bus, device, function, bar_id);
                    if (bar.address && (bar.type == INPUT_OUTPUT)) {
                        dev.port_base = (u32)bar.address;
                    }
                }
                dev.type = dev_type_by_class(&dev);
				pci_devs[count ++] = dev;
            }
        }
    }

    return 0;
}

pci_dev_t pci_get_device_desriptor(u8 bus, u8 device,
                                           u8 function) {
    pci_dev_t new_device = {0};

    new_device.bus = bus;
    new_device.device = device;
    new_device.function = function;

    new_device.vendor_id = pci_read(bus, device, function, 0x00);
    new_device.device_id = pci_read(bus, device, function, 0x02);

    new_device.class_id = pci_read(bus, device, function, 0x0b);
    new_device.subclass_id = pci_read(bus, device, function, 0x0a);
    new_device.interface_id = pci_read(bus, device, function, 0x09);
    new_device.revision_id = pci_read(bus, device, function, 0x08);

    new_device.interrupt = pci_read(bus, device, function, 0x3c);

    return new_device;
}

u32 pci_read_bar(pci_dev_t* pci_dev, int bar_id) {
    return _pci_do_read_bar(pci_dev->bus, pci_dev->device, pci_dev->function,
                            bar_id);
}

void init_pci() { pci_find_devices(); }

pci_dev_t* get_pci_bga()
{
    for (int i = 0; i < NR_PCI_DEV; ++ i) {
        if (pci_devs[i].type == DEVICE_DISPLAY) return &pci_devs[i];
    }
    return NULL;
}