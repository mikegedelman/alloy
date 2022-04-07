#ifndef _KERNEL_PCI_H
#define _KERNEL_PCI_H

#include <stdint.h>

typedef struct {
	uint16_t vendor_id;
	uint16_t device_id;

	uint8_t bus;
	uint8_t device;
	uint8_t fn;
} PCIDevice;

extern PCIDevice pci_devices[256];

void check_all_buses();
uint32_t pci_config_read(uint8_t bus, uint8_t slot, uint8_t fn, uint8_t offset);
void pci_config_write(uint8_t bus, uint8_t slot, uint8_t fn, uint8_t offset, uint32_t value);

#endif