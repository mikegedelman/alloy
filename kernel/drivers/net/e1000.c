#include <kernel/drivers/pci.h>
#include <kernel/mem/heap.h>
#include <kernel/mem/virtual.h>
#include <kernel/cpu.h>
#include <kernel/stdio.h>
#include <kernel/interrupts.h>

#include <kernel/drivers/net/e1000.h>

#include <stdint.h>
#include <stdbool.h>

#define VENDOR_ID 0x8086
#define DEVICE_ID 0x100E

#define REG_EEPROM 0x0014

typedef struct {
	uint32_t base_addr;
	uint8_t bar_type;
	uint8_t pci_bus;
	uint8_t pci_device;
	uint8_t pci_fn;
	bool has_eeprom;
} E1000Info;

static E1000Info e1000;
// static void* rtl_buf = NULL;

void e1000_handler(void *data) {
	printf("e1000 handler - data: %x\n", (uintptr_t) data);
}

void write32(uint32_t address, uint32_t value)
{
    (*((volatile uint32_t*)(address)))= value;
 
}

uint32_t read32 (uint32_t p_address)
{
    return *((volatile uint32_t*)(p_address));
 
}

void e1000_write(uint16_t address, uint32_t value) {
	if (e1000.bar_type == 0) {
		write32(e1000.base_addr + address, value);
	} else {
		outl(e1000.base_addr, address);
		outl(e1000.base_addr + 4, value);
	}
}

uint32_t e1000_read(uint16_t address) {
	if (e1000.bar_type == 0) {
		return read32(e1000.base_addr + address);
	} else {
		outl(e1000.base_addr, address);
		return inl(e1000.base_addr + 4);
	}
}

void detect_eeprom() {
	uint32_t val = 0;
	e1000_write(REG_EEPROM, 0x1);
	printf("Detecting eeprom...\n");

	for (int i = 0; i < 1000; i++) {
		val = e1000_read(REG_EEPROM);
		if (val & 0x10) {
			e1000.has_eeprom = true;
			printf("Detected e1000 EEPROM.\n");
			return;
		}
	}

	printf("e1000: no EEPROM detected.\n");
}

void e1000_init() {
	bool found = false;
	e1000.base_addr = 0;

	for (int i = 0; i < 256; i++) {
		if (pci_devices[i].vendor_id == VENDOR_ID && pci_devices[i].device_id == DEVICE_ID) {
			printf("Found Intel 82540EM Gigabit Ethernet Controller.\n");

			e1000.pci_bus = pci_devices[i].bus;
			e1000.pci_device = pci_devices[i].device;
			e1000.pci_fn = pci_devices[i].fn;

			found = true;
		}
	}

	if (!found) {
		printf("Error - couldn't find E1000 device. Aborting.\n");
		return;
	}

	uint32_t bar0 = pci_config_read(e1000.pci_bus, e1000.pci_device, e1000.pci_fn, 0x10);
	e1000.bar_type = bar0 & 1;
	if (e1000.bar_type == 0) {
		e1000.base_addr = (uint32_t)bar0 & 0xFFFFFFF0;
	} else {
		e1000.base_addr = (uint16_t)bar0 & 0xFFFFFFFC;	
	}
	

	printf("E1000 base address: %x - BAR0 Type: %x\n", e1000.base_addr, e1000.bar_type);
	printf("here...\n");
	detect_eeprom();
}