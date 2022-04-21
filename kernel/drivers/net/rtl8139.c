/*
 * https://wiki.osdev.org/RTL8139
 * https://github.com/szhou42/osdev/blob/master/src/kernel/drivers/rtl8139.c
 * https://www.cs.usfca.edu/~cruse/cs326f04/RTL8139_ProgrammersGuide.pdf
 */
#include <kernel/all.h>

#define ROK     (1<<0)
#define RER     (1<<1)
#define TOK     (1<<2)
#define TER     (1<<3)
#define TX_TOK  (1<<15)
#define CAPR 	0x38

enum RTL8139_registers {
  MAC0             = 0x00,       // Ethernet hardware address
  MAR0             = 0x08,       // Multicast filter
  TxStatus0        = 0x10,       // Transmit status (Four 32bit registers)
  TxAddr0          = 0x20,       // Tx descriptors (also four 32bit)
  RxBuf            = 0x30, 
  RxEarlyCnt       = 0x34, 
  RxEarlyStatus    = 0x36,
  ChipCmd          = 0x37,
  RxBufPtr         = 0x38,
  RxBufAddr        = 0x3A,
  IntrMask         = 0x3C,
  IntrStatus       = 0x3E,
  TxConfig         = 0x40,
  RxConfig         = 0x44,
  Timer            = 0x48,        // A general-purpose counter
  RxMissed         = 0x4C,        // 24 bits valid, write clears
  Cfg9346          = 0x50, 
  Config0          = 0x51, 
  Config1          = 0x52,
  FlashReg         = 0x54, 
  GPPinData        = 0x58, 
  GPPinDir         = 0x59, 
  MII_SMI          = 0x5A, 
  HltClk           = 0x5B,
  MultiIntr        = 0x5C, 
  TxSummary        = 0x60,
  MII_BMCR         = 0x62, 
  MII_BMSR         = 0x64, 
  NWayAdvert       = 0x66, 
  NWayLPAR         = 0x68,
  NWayExpansion    = 0x6A,
  
  // Undocumented registers, but required for proper operation
  FIFOTMS          = 0x70,        // FIFO Control and test
  CSCR             = 0x74,        // Chip Status and Configuration Register
  PARA78           = 0x78, 
  PARA7c           = 0x7c,        // Magic transceiver parameter register
};


enum ChipCmdBits {
  RxBufEmpty = 0x01,
  CmdTxEnb   = 0x04,
  CmdRxEnb   = 0x08,
  CmdReset   = 0x10,
};

enum RxStatusBits {
  RxStatusOK  = 0x0001,
  RxBadAlign  = 0x0002, 
  RxCRCErr    = 0x0004,
  RxTooLong   = 0x0008, 
  RxRunt      = 0x0010, 
  RxBadSymbol = 0x0020, 
  RxBroadcast = 0x2000,
  RxPhysical  = 0x4000, 
  RxMulticast = 0x8000, 
};

// enum rtl_8139_registers {
// 	tx_addr0 = 0x20,
// };

// Four TXAD register, you must use a different one to send packet each time(for example, use the first one, second... fourth and back to the first)
uint8_t TSAD_array[4] = {0x20, 0x24, 0x28, 0x2C};
uint8_t TSD_array[4] = {0x10, 0x14, 0x18, 0x1C};

typedef struct {
	uint16_t base_io;
	uint8_t pci_bus;
	uint8_t pci_device;
	uint8_t pci_fn;
	uint8_t mac_addr[6];
	uint8_t cur_tx;
} Rtl8319Info;

#define RX_BUF_SIZE (8192+16+1500)

static Rtl8319Info rtl_info;
static uint8_t rtl_buf[RX_BUF_SIZE];
static uint16_t current_packet_offset;


#define RX_READ_POINTER_MASK (~3)

void rtl_8139_receive_packet() {
	uint8_t cmd_in = inb(rtl_info.base_io + ChipCmd);
	while((cmd_in & RxBufEmpty) == 0) {
		uint8_t *rtl_buf_cur = rtl_buf + current_packet_offset;
		uint32_t rx_status = *(uint32_t*) rtl_buf_cur;

		uint16_t rx_size = rx_status >> 16;
		uint16_t packet_len = rx_size - 4;

		if (rx_status & (RxBadSymbol | RxRunt | RxTooLong | RxCRCErr | RxBadAlign)) {
			printf("8139: rx error: %x\n", rx_status);
			// TODO: reset the chip
			return;
		}
		if ((rx_status & (RxStatusOK)) == 0) {
			printf("8139: RxStatusOK == 0. Aborting.\n");
			return;
		}


		uint8_t packet[packet_len];
		memcpy(packet, rtl_buf_cur + 4, packet_len);
		ethernet_receive_packet(packet, packet_len);


		current_packet_offset = (current_packet_offset + rx_size + 4 + 3) & RX_READ_POINTER_MASK;

    if (current_packet_offset > RX_BUF_SIZE) {
        current_packet_offset -= RX_BUF_SIZE;
    }

    outw(rtl_info.base_io + CAPR, current_packet_offset - 0x10);
    cmd_in = inb(rtl_info.base_io + ChipCmd);
	}
}

void rtl_8139_handler(void *data) {
  uint16_t status = inw(rtl_info.base_io + IntrStatus);
	outw(rtl_info.base_io + IntrStatus, status);

  if(status & TOK) {
      printf("(rtl8139) Packet sent\n");
      outw(rtl_info.base_io + 0x3E, 0x5);
  }
  if (status & ROK) {
      printf("(rtl8139) Received packet\n");
      rtl_8139_receive_packet();
      outw(rtl_info.base_io + 0x3E, 0x5);
  }
}


void rtl_8139_tx(void *data, uint32_t len) {
	outl(rtl_info.base_io + TSAD_array[rtl_info.cur_tx], (uintptr_t)(data - BASE_VIRTUAL_ADDRESS));
	outl(rtl_info.base_io + TSD_array[rtl_info.cur_tx], len);
	rtl_info.cur_tx = (rtl_info.cur_tx + 1) % 4;
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

	// printf("Initializing the 8139.\n");

	// Turn the device on.
	outb(rtl_info.base_io + 0x52, 0x0);

	// Check its status register.
	outb(rtl_info.base_io + 0x37, 0x10);
 	while( (inb(rtl_info.base_io + 0x37) & 0x10) != 0) { }

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
    // printf("8139 interrupt line: %x\n", int_line);
    register_interrupt_handler(32 + int_line, rtl_8139_handler);
    pic_irq_clear_mask(32 + int_line);

    read_mac_addr();
}
