#ifndef _KERNEL_FS_H
#define _KERNEL_FS_H

#include <stdint.h>

void chs_read(uint16_t, uint32_t, uint16_t, uint16_t, char*);

#endif