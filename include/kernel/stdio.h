#ifndef _KERNEL_STDIO_H
#define _KERNEL_STDIO_H

#include <stddef.h>

void print_hex_byte(char);
void print_hex_buf(const char*, size_t);
void puts(const char*);

#endif
