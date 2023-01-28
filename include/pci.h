/*
** Copyright 2002-2003, Michael Noisternig. All rights reserved.
** Distributed under the terms of the NewOS License.
*/
#ifndef _PCI_H_
#define _PCI_H_
#include "type.h"
typedef struct {
    char prefetchable;
    u32 address;
    u8 type;
} bar_t;
#define INPUT_OUTPUT	0
#define MEMORY_MAPPED	1

struct device_desc_pci {
    u8 bus;
    u8 device;
    u8 function;
    u16 vendor_id;
    u16 device_id;
    u8 class_id;
    u8 subclass_id;
    u8 interface_id;
    u8 revision_id;
    u32 interrupt;
    u32 port_base;
	u32 type;
};
typedef struct device_desc_pci pci_dev_t;

enum DEVICES_TYPE {
    DEVICE_STORAGE = (1 << 0),
    DEVICE_SOUND = (1 << 1),
    DEVICE_INPUT_SYSTEMS = (1 << 2),
    DEVICE_NETWORK = (1 << 3),
    DEVICE_DISPLAY = (1 << 4),
    DEVICE_BUS_CONTROLLER = (1 << 5),
    DEVICE_BRIDGE = (1 << 6),
    DEVICE_CHAR = (1 << 7),
    DEVICE_RTC = (1 << 8),
    DEVICE_UNKNOWN = (1 << 9),
};

u32 pci_read(u16 bus, u16 device, u16 function, u32 offset);
void pci_write(u8 bus, u8 device, u8 function, u8 offset, u32 data);
pci_dev_t pci_get_device_desriptor(u8 bus, u8 device, u8 function);
pci_dev_t* get_pci_bga();
u32 pci_read_bar(pci_dev_t* pci_dev, int bar_id);
void init_pci();

#define NR_PCI_DEV  8

#endif
