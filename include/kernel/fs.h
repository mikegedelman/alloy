#ifndef _KERNEL_FS_H
#define _KERNEL_FS_H

#include <stdint.h>

typedef struct {
    char name[256];
    char cache[256];
    bool cache_dirty;
} GlobalFileDescriptor;

void chs_read(uint16_t, uint32_t, uint16_t, uint16_t, char*);

GlobalFileDescriptor* gft_get(int idx);
size_t gft_write(int idx, uint8_t *buf, size_t bytes);

#endif
