#include <stdint.h>
#include <kernel/cpu.h>
#include <kernel/stdio.h>
#include <kernel/drivers/pic8259.h>
#include <kernel/drivers/uart16550.h>

static void (*registered_interrupts[256]) (void*);

void init_interrupts() {
    for (int i = 0; i < 256; i++) {
        registered_interrupts[i] = NULL;
    }
}

void register_interrupt_handler(size_t irq_num, void (*fn)(void*)) {
    registered_interrupts[irq_num] = fn;
}

void send_eoi(uint32_t irq) {
    if (irq >= 8) {
        pic2_eoi();
    }

    pic1_eoi();
}

void isr_handler(uint32_t x, uint32_t info) {
    // term_putchar('?');)
    // char scancode;

    // if (x != 0x20) {
    //     printf("**INTERRUPT %x \n", x);
    // }

    if (x < 32) {
        printf("exception %x", x);
        while(1) { asm("hlt"); }
    }

    if (registered_interrupts[x] != NULL) {
        registered_interrupts[x]((void*) info);
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
    }

    send_eoi(x);
}

void _syscall(uint32_t x, void *data) {
    switch(x) {
        case 1:
            printf("%s", data);
            break;
        case 2:
            printf("exit\n");
            while(1) { asm("hlt"); }
            break;
    }
}
