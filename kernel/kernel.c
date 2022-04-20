#include <kernel/all.h>

extern int test();
extern void rust_main();

void exit_qemu() {
    outl(0xf4, 0x10);
}

void panic(const char *msg) {
    serial_write(&com1, "PANIC: ");
    serial_write(&com1, msg);
    serial_write(&com1, "\n");
    hlt();
}

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

/** Stuff to do after init. */
void kernel_tasks() {
    printf("Early init complete.\n");
    pci_scan();

    rtl_8139_init();

    register_udp_listener(68, receive_dhcp);
    dhcp_discover();
    IPAddress my_ip;
    while(1) {
        my_ip = get_my_ip();
        if (my_ip.parts[0] > 0) break;
    }
    printf("IP address set: ");
    print_ip((uint8_t*)&my_ip);
    printf("\n");

    while(1) {}
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

// MasterBootRecord *mbr = (MasterBootRecord*) buf;
// printf("partition type %x\n", mbr->partition_table[0].partition_type);
// printf("partition lba start %x\n", mbr->partition_table[0].lba_partition_start);

// Fat16Fs fat16fs = fat16_init(mbr->partition_table[0].lba_partition_start);
// printf("root dir lba %x\n", fat16fs.root_dir_offset);
// FatDirectoryEntry *entries_buf = heap_alloc(sizeof(FatDirectoryEntry) * 256);
// size_t num_entries = fat16_read_dir(&fat16fs, entries_buf);
// for (size_t i = 0; i < num_entries; i++) {
//     print_dir_entry(&entries_buf[i]);
// }


// FatFile *f = fat_open(&fat16fs, "USER.BIN");
// if (f == NULL) {
//     printf("Couldn't open USER.BIN.\n");
//     return;
// }

// uint8_t *file_buf = heap_alloc(f->size);
// fat_read(f, file_buf);

// exec((void*) file_buf);

// void print_dir_entry(FatDirectoryEntry *entry) {
//     char filename[12];
//     fat_render_filename(entry, filename);
//     printf("%s\n", filename);
// }
