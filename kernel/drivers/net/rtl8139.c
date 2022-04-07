#include <kernel/drivers/pci.h>
#include <kernel/mem/heap.h>
#include <kernel/mem/virtual.h>
#include <kernel/cpu.h>
#include <kernel/stdio.h>
#include <kernel/interrupts.h>

#include <stdint.h>

typedef struct {
	uint16_t base_io;
	uint8_t pci_bus;
	uint8_t pci_device;
	uint8_t pci_fn;
} Rtl8319Info;

static Rtl8319Info rtl_info;
static void* rtl_buf = NULL;

void rtl_8139_handler(void *data) {
	printf("8139 handler - data: %x\n", (uintptr_t) data);
}

void rtl_8139_init() {
	rtl_info.base_io = 0;

	for (int i = 0; i < 256; i++) {
		if (pci_devices[i].vendor_id == 0x10EC && pci_devices[i].device_id == 0x8139) {
			printf("Found RTL8319 NIC.\n");

			rtl_info.pci_bus = pci_devices[i].bus;
			rtl_info.pci_device = pci_devices[i].device;
			rtl_info.pci_fn = pci_devices[i].fn;
		}
	}

	uint32_t bar1 = pci_config_read(rtl_info.pci_bus, rtl_info.pci_device, rtl_info.pci_fn, 0x10);
	if ((bar1 & 1) == 0) {
		printf("ERROR: Unexpected RTL 8139 configuration - recieved a memory-space PCI base address. Aborting.\n");
		return;
	}

	rtl_info.base_io = (uint16_t)bar1 & 0xFFFC;
	printf("RTL 8139 base IO address: %x\n", rtl_info.base_io);

	uint32_t pci_command_reg = pci_config_read(rtl_info.pci_bus, rtl_info.pci_device, rtl_info.pci_fn, 0x4);
	if ((pci_command_reg & (1 << 2)) == 0) {
		printf("Enabling 8139 bus mastering.\n");
		pci_command_reg |= (1 << 2); // set the second bit

		pci_config_write(rtl_info.pci_bus, rtl_info.pci_device, rtl_info.pci_fn, 0x4, pci_command_reg);
	} else {
		printf("8139 Bus Mastering already enabled.\n");
	}

	// Verify that it worked
	io_wait();
	pci_command_reg = 0;
	pci_command_reg = pci_config_read(rtl_info.pci_bus, rtl_info.pci_device, rtl_info.pci_fn, 0x4);
	if ((pci_command_reg & (1 << 2)) == 0) {
		printf("Error enabling 8139 bus mastering..\n");
		return;
	}

	printf("Initializing the 8139.\n");

	// Turn the device on.
	outb(rtl_info.base_io + 0x52, 0x0);

	// Check its status register.
	outb(rtl_info.base_io + 0x37, 0x10);
 	while( (inb(rtl_info.base_io + 0x37) & 0x10) != 0) { }

 	// Send a *physical* memory pointer to a buffer that can be used for 
 	rtl_buf = heap_alloc(8192+16+1500);

	// TODO - this is a pretty sloppy way to convert the address to physical..
 	outl(rtl_info.base_io + 0x30, (uintptr_t)(rtl_buf - BASE_VIRTUAL_ADDRESS));

 	 // Sets the TOK and ROK bits high
 	outw(rtl_info.base_io + 0x3C, 0x0005);

    // (1 << 7) is the WRAP bit, 0xf is AB+AM+APM+AAP
    // TODO - AAP-promiscuous mode, I probably don't need that.
    outl(rtl_info.base_io + 0x44, 0xf | (1 << 7));

    // Sets the RE and TE bits high
    outb(rtl_info.base_io + 0x37, 0x0C);

    uint32_t int_line = pci_config_read(rtl_info.pci_bus, rtl_info.pci_device, rtl_info.pci_fn, 0x3C) & 0xFF;
    printf("8139 interrupt line: %x\n", int_line);
    register_interrupt_handler(32 + int_line, rtl_8139_handler);

}