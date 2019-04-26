#ifndef _KERNEL_TERM_H
#define _KERNEL_TERM_H

void term_clear();
void term_init();
void term_putchar(char);
void term_write(const char* s);
void term_puts(const char* s);

#endif
