#include <stdint.h>
#include <stddef.h>

#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_COLOR 0x0F

uint16_t *term_buffer;
size_t term_row;
size_t term_column;

//void term_putchar(char);

void term_putentryat(char c, size_t x, size_t y) {
	term_buffer[y * VGA_WIDTH + x] = c | (VGA_COLOR << 8);
}

void term_clear() {
   for (size_t y = 0; y < VGA_HEIGHT; y++) {
       for (size_t x = 0; x < VGA_WIDTH; x++) {
           term_putentryat(' ', x, y);
       }
   }
}

void term_shift_up() {
   for (uint16_t row = 1; row < VGA_HEIGHT; row++) {
       for (uint16_t col = 0; col < VGA_WIDTH; col++) {
           uint16_t from_col = (row * VGA_WIDTH) + col;

           term_buffer[from_col - VGA_WIDTH] = term_buffer[from_col];
       }
   }

   for (uint16_t clearcol = 0; clearcol < VGA_WIDTH; clearcol++) {
       term_putentryat(' ', clearcol, VGA_HEIGHT - 1);
   }
}

void term_newline() {
   if (term_row == (VGA_HEIGHT - 1)) {
       term_shift_up();
   } else {
       term_row++;
   }
   term_column = 0;
}

void term_putchar(char c) {
    if (c == '\n' || c == '\r') {
        term_newline();
    } else {
    	term_putentryat(c, term_column, term_row);
        term_column++;

        if (term_column == VGA_WIDTH) {
            term_newline();
        }
    }
}

/* Assumes s is null-terminated, which might be a bad idea */
void term_puts(const char* s) {
    term_newline();

    size_t i = 0;
    while (s[i]) {
        term_putchar(s[i]);
        i++;
    }
}

void term_init() {
    term_buffer = (uint16_t*) 0xB8000;
    term_row = 0;
    term_column = 0;

    term_clear();
    //term_putentryat('A', 10, 10);
}

void kernel_main() {
     term_init();
     // term_putentryat('C', 0, 0);

    // term_putentryat('D', 2, 5);
    // term_putchar2('H');
    // return;
    term_puts("Hi\n");
    term_puts("Hi\n");
    term_puts("Hi\n");
    term_puts("Hi\n");
    term_puts("Hi\n");
    term_puts("Hi\n");
    term_puts("Hi\n");
    term_puts("Hi\n");
    term_puts("Hi\n");
    term_puts("Hi\n");
    term_puts("Hi\n");
    term_puts("Hi\n");
    term_puts("Hi\n");
    term_puts("Hi\n");
    term_puts("Hi\n");
    term_puts("Hi\n");
    term_puts("Hi\n");
    term_puts("Hi\n");
    term_puts("Hi\n");
    term_puts("Hi\n");
    term_puts("Hi\n");
    term_puts("Hi\n");
    term_puts("Hi\n");
    term_puts("Hi\n");
    term_puts("Hi\n");
    term_puts("Hi\n");
    term_puts("Hi\n");
    term_puts("Kernel\n");
    term_puts("Kernel\n");
    term_puts("Kernel\n");
    term_puts("Kernel\n");
    term_puts("Kernel\n");
    term_puts("final line");
    //term_puts("another line");
}
