#include <stdint.h>

typedef struct __attribute__((__packed__)) {
	uint16_t    isr_low;      // The lower 16 bits of the ISR's address
	uint16_t    kernel_cs;    // The GDT segment selector that the CPU will load into CS before calling the ISR
	uint8_t     reserved;     // Set to zero
	uint8_t     attributes;   // Type and attributes; see the IDT page
	uint16_t    isr_high;     // The higher 16 bits of the ISR's address
} IdtEntry;

typedef struct __attribute__((__packed__)) {
	uint16_t	limit;
	uint32_t	base;
} IdtPtr;

__attribute__((aligned(0x10))) 
static IdtEntry idt[256]; // Create an array of IDT entries; aligned for performance
static IdtPtr idt_ptr;

extern void *isr_handler_table[];

void idt_set_descriptor(int vector, void *isr) { // }, uint8_t flags) {
    IdtEntry *descriptor = &idt[vector];
 
    descriptor->isr_low        = (uint32_t)isr & 0xFFFF;
    descriptor->kernel_cs      = 0x08; // this value can be whatever offset your kernel code selector is in your GDT
    descriptor->reserved       = 0;
    descriptor->attributes     = 0x8E;
    descriptor->isr_high       = (uint32_t)isr >> 16;
}

void init_idt() {
    for (int i = 0; i < 256; i++) {
        idt_set_descriptor(i, isr_handler_table[i]);
    }

    idt_ptr.limit = (sizeof(IdtEntry) * 256) - 1;
    idt_ptr.base = (uint32_t) idt;

    __asm__ volatile (
        "lidt (%0)" ::
        "m" (idt_ptr)
    );
}

// __attribute__((noreturn))
// // void exception_handler(void);
// void exception_handler() {
//     __asm__ volatile ("cli; hlt"); // Completely hangs the computer
// }