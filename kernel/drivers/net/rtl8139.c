// https://wiki.osdev.org/RTL8139
// https://github.com/szhou42/osdev/blob/master/src/kernel/drivers/rtl8139.c
#include <kernel/drivers/pci.h>
#include <kernel/drivers/pic8259.h>
#include <kernel/mem/heap.h>
#include <kernel/mem/virtual.h>
#include <kernel/cpu.h>
#include <kernel/stdio.h>
#include <kernel/interrupts.h>
#include <kernel/string.h>
#include <kernel/inet.h>

#include <kernel/net/link.h>

#include <stdint.h>

#define ROK     (1<<0)
#define RER     (1<<1)
#define TOK     (1<<2)
#define TER     (1<<3)
#define TX_TOK  (1<<15)

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

static Rtl8319Info rtl_info;
static uint8_t* rtl_buf = NULL;


void rtl_8139_receive_packet() {
	uint16_t *buf_as_u16 = (uint16_t*)rtl_buf;
	// printf("header: %x\n", buf_as_u16[0]);
	size_t packet_len = buf_as_u16[1];
	printf("packet length: %x\n", packet_len);

	uint8_t *packet = (uint8_t*)heap_alloc(packet_len);
	memcpy(packet, rtl_buf + 4, packet_len);
	ethernet_receive_packet(packet, packet_len);
}

void rtl_8139_handler(void *data) {
	cli();
	// printf("8139 handler - data: %x\n", (uintptr_t) data);
	    //qemu_printf("RTL8139 interript was fired !!!! \n");
    uint16_t status = inw(rtl_info.base_io + 0x3E);

    if(status & TOK) {
        printf("(rtl8139) Packet sent\n");
    }
    if (status & ROK) {
        printf("(rtl8139) Received packet\n");
        rtl_8139_receive_packet();
    }

    outw(rtl_info.base_io + 0x3E, 0x5);
    sti();
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
    pic_irq_clear_mask(32 + int_line);

    read_mac_addr();
}
