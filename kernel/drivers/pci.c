#include <kernel/drivers/pci.h>
#include <stdint.h>

#include <kernel/cpu.h>
#include <kernel/stdio.h>

PCIDevice pci_devices[256];

uint32_t pci_config_read(uint8_t bus, uint8_t slot, uint8_t fn, uint8_t offset) {
    uint32_t address;
    uint32_t lbus  = (uint32_t)bus;
    uint32_t lslot = (uint32_t)slot;
    uint32_t lfunc = (uint32_t)fn;
    uint32_t tmp = 0;
 
    // Create configuration address as per Figure 1
    address = (uint32_t)((lbus << 16) | (lslot << 11) |
              (lfunc << 8) | (offset & 0xFC) | ((uint32_t)0x80000000));
 
    // Write out the address
    outl(0xCF8, address);
    // Read in the data
    // (offset & 2) * 8) = 0 will choose the first word of the 32-bit register
    tmp = inl(0xCFC); // >> ((offset & 2) * 8)) & 0xFFFF);
    return tmp;
}

void pci_config_write(uint8_t bus, uint8_t slot, uint8_t fn, uint8_t offset, uint32_t value) {
    uint32_t address;
    uint32_t lbus  = (uint32_t)bus;
    uint32_t lslot = (uint32_t)slot;
    uint32_t lfunc = (uint32_t)fn;
 
    // Create configuration address as per Figure 1
    address = (uint32_t)((lbus << 16) | (lslot << 11) |
              (lfunc << 8) | (offset & 0xFC) | ((uint32_t)0x80000000));
 
    // Write out the address
    outl(0xCF8, address);
    outl(0xCFC, value);
}

void print_type(uint8_t bus, uint8_t device, uint8_t fn) {
	uint32_t header_0x8 = pci_config_read(bus, device, fn, 0x8);
	uint32_t class_code = header_0x8 >> 24;
	uint32_t subclass = (header_0x8 >> 16) & 0xFF;

    switch (class_code) {
        case 0x0:
            printf("Unclassified PCI Device");
            switch (subclass) {
                case 0x0:
                    printf(" - Non-VGA-Compatible Unclassified Device\n");
                    break;
                case 0x1:
                    printf(" - VGA-Compatible Unclassified Device\n");
                    break;
                default:
                    printf("\n");
            }
            break;
        case 0x1:
            printf("Mass Storage Controller\n");
            break;
        case 0x2:
            printf("Network Controller");
            switch (subclass) {
                case 0x0:
                    printf(" - Ethernet\n");
                    break;
                default:
                    printf("\n");
            }
            break;
        case 0x3:
            printf("Display Controller");
            switch (subclass) {
                case 0x0:
                    printf(" - VGA\n");
                    break;
                default:
                    printf("\n");
            }
            break;
        case 0x4:
            printf("Multimedia Controller\n");
            break;
        case 0x5:
            printf("Memory Controller\n");
            break;
        case 0x6:
            printf("Bridge");
            switch (subclass) {
                case 0x0:
                    printf(" - Host\n");
                    break;
                case 0x1:
                    printf(" - ISA\n");
                    break;
                case 0x80:
                    printf(" - Other\n");
                    break;
                default:
                    printf("\n");
            }
            break;
        default:
            printf("Unsupported/Unknown Device\n");
    }
}

void check_device(uint8_t bus, uint8_t device) {
    uint8_t fn = 0;
    size_t pci_device_num = 0; // for writing to pci_devices

    uint32_t header_0 = pci_config_read(bus, device, fn, 0);
    // uint16_t device_id = pci_config_read_word(bus, device, fn, 2);
    // uint16_t vendor_id = get_vendor_id(bus, device, fn);
    uint32_t vendor_id = header_0 & 0xFFFF;
    uint32_t device_id = header_0 >> 16;
    if (vendor_id == 0xFFFF) return; // Device doesn't exist
    // printf("device_id: %x, vendor_id: %x\n\t", device_id, vendor_id);

    uint32_t header_0xc = pci_config_read(bus, device, fn, 0xC);
    uint32_t header_type = (header_0xc >> 16) & 0xFF;
    // printf("Header type: %x\n\t", header_type)

    // switch (header_type) {
    //     case 0x1:
    //         printf("PCI-to-PCI Bridge\n");
    //         return;
    //     case 0x2:
    //         printf("PCI-to-CardBus Bridge\n");
    //         return;
    // }

    if ((header_type & 0x80) != 0) {
        // printf("Multi-function device\n\t");
        for (int i = 0; i < 32; i++) {
            // print_type(bus, device, i);
            // printf("\t");

            pci_devices[pci_device_num].vendor_id = vendor_id;
            pci_devices[pci_device_num].device_id = device_id;
            pci_devices[pci_device_num].bus = bus;
            pci_devices[pci_device_num].device = device;
            pci_devices[pci_device_num].fn = i;
        }
    } else {
        // print_type(bus, device, 0);
        pci_devices[pci_device_num].vendor_id = vendor_id;
        pci_devices[pci_device_num].device_id = device_id;
        pci_devices[pci_device_num].bus = bus;
        pci_devices[pci_device_num].device = device;
        pci_devices[pci_device_num].fn = 0;
    }
    // printf("\n");

    // check_fn(bus, device, fn);
    // headerType = getHeaderType(bus, device, fn);
    // if( (headerType & 0x80) != 0) {
    //     // It's a multi-fn device, so check remaining fns
    //     for (fn = 1; fn < 8; fn++) {
    //         if (get_vendor_id(bus, device, fn) != 0xFFFF) {
    //             check_fn(bus, device, fn);
    //         }
    //     }
    // }
}

void check_all_buses() {
	for (uint16_t bus = 0; bus < 256; bus++) {
		for (uint8_t device = 0; device < 32; device++) {
			check_device(bus, device);
		}
	}
}

// void check_fn(uint8_t bus, uint8_t device, uint8_t function) {

// }
