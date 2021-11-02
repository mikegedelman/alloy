#pragma once

#include <stddef.h>

void heap_init();
void *heap_alloc(size_t);
void heap_free(void*);
