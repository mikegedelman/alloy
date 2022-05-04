#include <kernel/all.h>

#define assert(expr, msg) if (!(expr)) { panic(msg); }

void early_init() {
    init_gdt();
    init_idt();
    pic_remap(32, 40);
    sti();
    term_disable_cursor();
    serial_init();
    term_init();
    ata_init();
    init_interrupts();
}

void setup_memory(MultibootInfo *multiboot_info, uint32_t magic) {
    assert(magic == MULTIBOOT_MAGIC, "Invalid multiboot magic number");
    load_multiboot_mmap(multiboot_info);
    int num_free = get_num_free();
    int free_mb = (num_free * 0x1000) / 1024 / 1024;

    printf("Found %d free physical frames. (%d MB)\n", num_free, free_mb);

    virtualmem_init();
    heap_init();
}

//void tcp_listener(uint8_t *data, size_t _data_len) {
//    printf("%s", (char *) data);
//}



void print_dir_entry(FatDirectoryEntry *entry) {
    char filename[12];
    fat_render_filename(entry, filename);
    printf("%s\n", filename);
}


/** Stuff to do after init. */
void kernel_tasks() {
    printf("Early init complete.\n");

    uint8_t buf[512];
    int bytes_read = ata_read(&ata1, ATA_MASTER, 0, 512, buf);
    printf("%d bytes read from ata1.\n", bytes_read);
    MasterBootRecord *mbr = (MasterBootRecord*) buf;
    printf("partition type %x\n", mbr->partition_table[0].partition_type);
    printf("partition lba start %x\n", mbr->partition_table[0].lba_partition_start);

    Fat16Fs fat16fs = fat16_init(mbr->partition_table[0].lba_partition_start);
    printf("root dir lba %x\n", fat16fs.root_dir_offset);
    FatDirectoryEntry *entries_buf = heap_alloc(sizeof(FatDirectoryEntry) * 256);
    size_t num_entries = fat16_read_dir(&fat16fs, entries_buf);
    for (size_t i = 0; i < num_entries; i++) {
        print_dir_entry(&entries_buf[i]);
    }


    FatFile *f = fat_open(&fat16fs, "USER.EXE");
    if (f == NULL) {
        printf("Couldn't open USER.EXE.\n");
        return;
    }

    printf("USER.EXE: %d bytes\n", f->size);
    uint8_t *file_buf = heap_alloc(f->size);
    fat_read(f, file_buf);

    exec((void*) file_buf);

}

void kernel_main(MultibootInfo *multiboot_info, uint32_t magic) {
    early_init();
    setup_memory(multiboot_info, magic);

    kernel_tasks();

    exit_qemu();
    hlt();
}


// Other stuff - previous testing
// int *some_mem = heap_alloc(1024);
// printf("Got a pointer to heap memory at 0x%x\n", some_mem);
// some_mem[0] = 0x1;

// uint8_t buf[512];
// int bytes_read = ata_read(&ata1, ATA_MASTER, 0, 512, buf);
// printf("%d bytes read from ata1.\n", bytes_read);



// Networking stuff
//    pci_scan();
//
//    rtl_8139_init();
//
//    register_udp_listener(68, receive_dhcp);
//    dhcp_discover();
//    IPAddress my_ip;
//    while(1) {
//        my_ip = get_my_ip();
//        if (my_ip.parts[0] > 0) break;
//    }
//    printf("IP address set: ");
//    print_ip(&my_ip);
//    printf("\n");

//    IPAddress dest = new_ip(54, 192, 58, 2);
//    uint16_t host_port = tcp_open(dest, 80, tcp_listener);
//    printf("Opened a TCP connection on host port %u\n", host_port);
//    const char* http_req = "GET /index.html HTTP/1.1\r\nConnection: Close\r\n\r\n";
//    tcp_send(host_port, http_req, strlen(http_req));
//    while(tcp_is_open(host_port)) {}
