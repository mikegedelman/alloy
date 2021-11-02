#ifndef _KERNEL_STDIO_H
#define _KERNEL_STDIO_H

#include <stddef.h>

void putchar(char);
void puts(const char*);
void printf(const char *, ...);
char* convert(unsigned int, int);

#endif
