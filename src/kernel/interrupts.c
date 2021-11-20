#include <stdint.h>
#include <kernel/cpu.h>
#include <kernel/stdio.h>
#include <kernel/drivers/pic8259.h>
#include <kernel/drivers/uart16550.h>

void send_eoi(uint32_t irq) {
    if (irq >= 8) {
        pic2_eoi();
    }

    pic1_eoi();
}

void _syscall(uint32_t x, void *data) {
    printf("syscall %x", x);
    if (x == 2) {
        while(1) { hlt(); }
    }
}

void isr_handler(uint32_t x, uint32_t info) {
    // term_putchar('?');)
    // char scancode;
    if (x < 32) {
        printf("exception %x", x);
    }
    switch (x) {
        case 32:
            // term_putchar('.');
            // serial_write(&com1, ".");
            break;
        case 33:
            // scancode = inb(0x60);
            // serial_write(&com1, "?");
            inb(0x60);
            break;
        case 0x80:
            printf("this shouldn't happen.\n");
    }
    send_eoi(x);
}


