#include <kernel/all.h>

static GlobalFileDescriptor global_file_table[16];

GlobalFileDescriptor* gft_get(int idx) {
    return &global_file_table[idx];
}

size_t gft_write(int idx, uint8_t *buf, size_t bytes) {
    if (idx == 1) {
        for (int i = 0; i < bytes; i++) {
            putchar(buf[i]);
        }
        return bytes;
    }
    GlobalFileDescriptor *gfd = gft_get(idx);
    size_t bytes_to_write = MIN(bytes, 256);
    for (int i = 0; i < bytes_to_write; i++) {
        gfd->cache[i] = buf[i];
    }
    gfd->cache_dirty = true;
    return bytes_to_write;
}

// void gft_init() {

// }

// void chs_read(uint16_t head,
//               uint32_t cyl,
//               uint16_t sector,
//               uint16_t num_sectors,
//               char* buf) {
//    head = (head & 0x0F) | 0xa0;
//    outb(0x1f6, head);
//    outb(0x1f3, sector);
//    outb(0x1f4, cyl & 0xFFFF);
//    outb(0x1f5, cyl >> 16);
//    outb(0x1f2, num_sectors);
//    outb(0x1f7, 0x20); // read with retry

//    uint8_t in, test;
//    do {
//        in = inb(0x1f7);
//        test = in & 8;
//    } while (test != 0);

//    __asm__ __volatile__ ("mov %1, %%ecx; \n\t"
//                          "mov %0, %%edi; \n\t"
//                          "mov $0x1f0, %%dx; \n\t"
//                          "rep; insw; \n\t" ::
//                          // input
//                          "m" (buf),
//                          "r" (256)
//                          :
//                          // clobber
//                          "ecx", "edi", "edx", "memory");
// }
