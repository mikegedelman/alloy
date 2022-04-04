#include <stdint.h>
#include <stdbool.h>
#include <kernel/arch/x86.h>
#include <kernel/term.h>
#include <kernel/stdio.h>
#include <kernel/string.h>
#include <kernel/fs.h>
#include <kernel/drivers/uart16550.h>
#include <kernel/drivers/pic8259.h>
#include <kernel/cpu.h>
#include <kernel/test.h>
#include <kernel/multiboot.h>
#include <kernel/mem/physical.h>
#include <kernel/mem/virtual.h>
#include <kernel/mem/heap.h>
#include <kernel/drivers/ata.h>
#include <kernel/fs/volume.h>
#include <kernel/fs/fat.h>

extern int test();

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

void print_dir_entry(FatDirectoryEntry *entry) {
    char filename[12];
    fat_render_filename(entry, filename);
    printf("%s\n", filename);
}

/** Stuff to do after init. */
void kernel_tasks() {
    serial_write(&com1, "Welcome to os.\n");
    term_puts("Welcome to os.\n");

    int *some_mem = heap_alloc(1024);
    printf("Got a pointer to heap memory at 0x%x\n", some_mem);
    some_mem[0] = 0x1;

    int *some_mem2 = heap_alloc(1024);
    printf("Got a pointer to heap memory at 0x%x\n", some_mem2);
    some_mem2[0] = 0x2;

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

    FatFile *f = fat_open(&fat16fs, "USER.BIN");
    if (f == NULL) {
        printf("Couldn't open USER.BIN.\n");
        return;
    }

    uint8_t *file_buf = heap_alloc(f->size);
    fat_read(f, file_buf);

    asm volatile(
        "jmp *(%0)" :: "r" (&file_buf)
    );

    
}

void kernel_main(MultibootInfo *multiboot_info, uint32_t magic) {
    early_init();
    setup_memory(multiboot_info, magic);

    #ifdef TEST
    run_tests();
    #else
    kernel_tasks();
    #endif

    hlt();
}
