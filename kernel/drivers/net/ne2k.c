#include <kernel/drivers/pci.h>
#include <kernel/mem/heap.h>
#include <kernel/mem/virtual.h>
#include <kernel/cpu.h>
#include <kernel/stdio.h>
#include <kernel/interrupts.h>

#include <kernel/drivers/net/ne2k.h>

#include <stdint.h>

const uint16_t VENDOR_ID = 0x10EC;
const uint16_t DEVICE_ID = 0x8029;

typedef struct {
	uint16_t base_io;
	uint8_t pci_bus;
	uint8_t pci_device;
	uint8_t pci_fn;
} NE2kInfo;



// TODO move this
typedef struct __attribute__((__packed__)) {
	uint16_t hardware_type; // Ethernet: 0x1
	uint16_t protocol_type; // 
	uint8_t hw_addr_len; // MAC: 6
	uint8_t proto_addr_len; // IPv4 = 4
	uint16_t opcode;
	uint8_t src_hw_addr[6];
	uint8_t src_proto_addr[4];
	uint8_t dest_hw_addr[6];
	uint8_t dest_proto_addr[4];
} ARPPacket;

// typedef struct {
	
// } EthernetPacket;

static NE2kInfo ne2k;
// static void* rtl_buf = NULL;

void ne2k_handler(void *data) {
	printf("ne2k handler - data: %x\n", (uintptr_t) data);
}

void ne2k_writeb(uint16_t offset, uint8_t value) {
	outb(ne2k.base_io + offset, value);
}

void ne2k_init() {
	ne2k.base_io = 0;

	for (int i = 0; i < 256; i++) {
		if (pci_devices[i].vendor_id == VENDOR_ID && pci_devices[i].device_id == DEVICE_ID) {
			printf("Found ne2k NIC.\n");

			ne2k.pci_bus = pci_devices[i].bus;
			ne2k.pci_device = pci_devices[i].device;
			ne2k.pci_fn = pci_devices[i].fn;
		}
	}

	uint32_t bar0 = pci_config_read(ne2k.pci_bus, ne2k.pci_device, ne2k.pci_fn, 0x10);
	printf("bar0: %x\n", bar0);
	if ((bar0 & 1) == 0) {
		printf("ERROR: Unexpected ne2k configuration - recieved a memory-space PCI base address. Aborting.\n");
		return;
	}

	ne2k.base_io = (uint16_t)bar0 & 0xFFFC;
	printf("ne2k base IO address: %x\n", ne2k.base_io);


	uint8_t prom[32];
	outb(ne2k.base_io, (1 << 5) | 1);	// page 0, no DMA, stop
	outb(ne2k.base_io + 0x0E, 0x49);		// set word-wide access
	outb(ne2k.base_io + 0x0A, 0);		// clear the count regs
	outb(ne2k.base_io + 0x0B, 0);
	outb(ne2k.base_io + 0x0F, 0);		// mask completion IRQ
	outb(ne2k.base_io + 0x07, 0xFF);
	outb(ne2k.base_io + 0x0C, 0x20);		// set to monitor
	outb(ne2k.base_io + 0x0D, 0x02);		// and loopback mode.
	outb(ne2k.base_io + 0x0A, 32);		// reading 32 bytes
	outb(ne2k.base_io + 0x0B, 0);		// count high
	outb(ne2k.base_io + 0x08, 0);		// start DMA at 0
	outb(ne2k.base_io + 0x09, 0);		// start DMA high
	outb(ne2k.base_io, 0x0A);		// start the read

	for (int i=0; i<32; i++) {
	  prom[i] = inb(ne2k.base_io + 0x10);
	};


	printf("ne2k MAC: ");
	//program the PAR0..PAR5 registers to listen for packets to our MAC address!		
	outb(ne2k.base_io, 0x61);
	for (int i=0; i<6; i++)
	{
		outb(ne2k.base_io + 0x01 + i, prom[i]);
	    // writeRegister(nif, 1, 0x01+i, prom[i]);
		printf("%x ", prom[i]);
	};
	printf("\n");

	uint32_t int_line = pci_config_read(ne2k.pci_bus, ne2k.pci_device, ne2k.pci_fn, 0x3C); // & 0xFF;
    printf("ne2k interrupt line: %x\n", int_line);
    register_interrupt_handler(32 + int_line, ne2k_handler);
    register_interrupt_handler(32 + 9, ne2k_handler);

    // ARPPacket arp_request;
    // arp_request.hardware_type = 0x1;
    // arp_request.protocol_type = 0x0800;
    // arp_request.hw_addr_len = 6;
    // arp_request.proto_addr_len = 4;
    // arp_request.opcode = 0x0001;
    // for (int i = 0; i < 6; i++) {
    // 	arp_request.src_hw_addr[i] = prom[i];
    // }
    // arp_request.src_proto_addr = { 0, 0, 0, 0};
    // arp_request.dest_proto_addr = { 192, 168, 10, 1};
    // arp_request.dest_hw_addr = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };


}