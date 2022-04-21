#ifndef _KERNEL_STRING_H
#define _KERNEL_STRING_H

#include <stddef.h>
#include <stdbool.h>

void *memcpy(void *dest, const void *src, size_t n);
int memcmp(uint8_t *a, const uint8_t *b, size_t n);
bool strcmp(const char *a, const char *b);
size_t strlen(const char *s);

#endif