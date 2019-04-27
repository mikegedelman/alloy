#include <stdint.h>
#include <stddef.h>
#include <kernel/term.h>

#include <kernel/stdio.h>


static inline char nibble_to_char(uint8_t nib) {
    if (nib > 9) {
        return 'a' + (nib - 10);
    } else {
        return '0' + nib;
    }
}

void print_hex_byte(char byte) {
   uint8_t low = byte & 0x0F;
   uint8_t high = (byte >> 4) & 0x0F;
   term_putchar(nibble_to_char(high));
   term_putchar(nibble_to_char(low));

}

void print_hex_buf(const char *bytes, size_t sz) {
    for (size_t i = 0; i < sz; i++) {
        print_hex_byte(bytes[i]);
        term_putchar(' ');
    }
}

void puts(const char* s) {
    term_puts(s);
}
