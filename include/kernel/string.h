#ifndef _KERNEL_STRING_H
#define _KERNEL_STRING_H

#include <stddef.h>
#include <stdbool.h>

void *memcpy(void *dest, const void *src, size_t n);
bool strcmp(char *a, char *b);

#endif