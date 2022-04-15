#include <kernel/drivers/pci.h>
#include <kernel/mem/heap.h>
#include <kernel/mem/virtual.h>
#include <kernel/cpu.h>
#include <kernel/stdio.h>
#include <kernel/interrupts.h>
#include <kernel/string.h>
#include <kernel/inet.h>

#include <kernel/net/link.h>

#include <stdint.h>

enum rtl_8139_registers {
	tx_addr0 = 0x20,
};

typedef struct {
	uint16_t base_io;
	uint8_t pci_bus;
	uint8_t pci_device;
	uint8_t pci_fn;
	uint8_t mac_addr[6];
} Rtl8319Info;

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


typedef struct __attribute__((__packed__)) {
	uint8_t dest_mac[6];
	uint8_t src_mac[6];
	uint16_t ethertype;
	// void *data;
} Etherheader;

static Rtl8319Info rtl_info;
static void* rtl_buf = NULL;

void rtl_8139_handler(void *data) {
	printf("8139 handler - data: %x\n", (uintptr_t) data);
}


void rtl_8139_tx(void *data, uint32_t len) {
	outl(rtl_info.base_io + 0x20, (uintptr_t)(data - BASE_VIRTUAL_ADDRESS));
	outl(rtl_info.base_io + 0x10, len);
}

void read_mac_addr() {
    uint32_t mac_part1 = inl(rtl_info.base_io + 0x00);
    uint16_t mac_part2 = inw(rtl_info.base_io + 0x04);
    rtl_info.mac_addr[0] = mac_part1 >> 0;
    rtl_info.mac_addr[1] = mac_part1 >> 8;
    rtl_info.mac_addr[2] = mac_part1 >> 16;
    rtl_info.mac_addr[3] = mac_part1 >> 24;

    rtl_info.mac_addr[4] = mac_part2 >> 0;
    rtl_info.mac_addr[5] = mac_part2 >> 8;
    printf("MAC Address: %x:%x:%x:%x:%x:%x\n", rtl_info.mac_addr[0], rtl_info.mac_addr[1], rtl_info.mac_addr[2], rtl_info.mac_addr[3], rtl_info.mac_addr[4], rtl_info.mac_addr[5]);
}

MacAddress rtl_8139_get_mac_addr() {
	return new_mac_from_array(rtl_info.mac_addr);
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

    read_mac_addr();

    // ARPPacket arp_request;
    // arp_request.hardware_type = htons(0x1);
    // arp_request.protocol_type = htons(0x0800);
    // arp_request.hw_addr_len = 6;
    // arp_request.proto_addr_len = 4;
    // arp_request.opcode = htons(0x0001);
    // for (int i = 0; i < 6; i++) {
    // 	arp_request.src_hw_addr[i] = rtl_info.mac_addr[i];
    // }
    // for (int i = 0; i < 4; i++) {
    // 	arp_request.src_proto_addr[i] = 0;
    // }
    // arp_request.dest_proto_addr[0] = 192;
    // arp_request.dest_proto_addr[1] = 168;
    // arp_request.dest_proto_addr[2] = 0;
    // arp_request.dest_proto_addr[3] = 1;
    // for (int i = 0; i < 6; i++) {
    // 	arp_request.dest_hw_addr[i] = 0xFF;
    // }
    // printf("hw address len: %x\n", arp_request.hw_addr_len);

    // Etherheader header;

    // for (int i = 0; i < 6; i++) {
    // 	header.src_mac[i] = rtl_info.mac_addr[i];
    // }
    // for (int i = 0; i < 6; i++) {
    // 	header.dest_mac[i] = 0xFF;
    // }
    // header.ethertype = htons(0x0806);

    // uint8_t arp_buf[60];
    // for (int i = 0; i < 60; i++) {
    // 	arp_buf[i] = 0;
    // }
    // memcpy(arp_buf, &header, sizeof(Etherheader));
    // memcpy(arp_buf + sizeof(Etherheader), &arp_request, sizeof(arp_request));

    // rtl_8139_tx(arp_buf, 60);
}
