#include <stdint.h>
#include <kernel/fs.h>

inline void outb (uint16_t port, uint8_t val) {
    __asm__ __volatile__ ("outb %1, %0" : : "dN" (port), "a" (val));
}

inline uint16_t inb (uint16_t port) {
    unsigned char rv;
    __asm__ __volatile__ ("inb %1, %0" : "=a" (rv) : "dN" (port));
    return rv;
}

void chs_read(uint16_t head,
              uint32_t cyl,
              uint16_t sector,
              uint16_t num_sectors,
              char* buf) {
   head = (head & 0x0F) | 0xa0;
   outb(0x1f6, head);
   outb(0x1f3, sector);
   outb(0x1f4, cyl & 0xFFFF);
   outb(0x1f5, cyl >> 16);
   outb(0x1f2, num_sectors);
   outb(0x1f7, 0x20); // read with retry

   uint8_t in, test;
   do {
       in = inb(0x1f7);
       test = in & 8;
   } while (test != 0);

   __asm__ __volatile__ ("mov %1, %%ecx; \n\t" 
                         "mov %0, %%edi; \n\t" 
                         "mov $0x1f0, %%dx; \n\t" 
                         "rep; insw; \n\t" ::
                         // input
                         "m" (buf),
                         "r" (256)
                         : 
                         // clobber
                         "ecx", "edi", "edx", "memory");
}
