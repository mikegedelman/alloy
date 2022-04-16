#ifndef _KERNEL_HEAP_H
#define _KERNEL_HEAP_H

#include <stddef.h>

void heap_init();
void *heap_alloc(size_t);
void heap_free(void*);

#endif