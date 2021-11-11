#pragma once

#include <stdint.h>

static inline void outb(uint16_t port, uint8_t val)
{
    asm volatile ( "outb %0, %1" : : "a"(val), "Nd"(port) );
    /* There's an outb %al, $imm8  encoding, for compile-time constant port numbers that fit in 8b.  (N constraint).
     * Wider immediate constants would be truncated at assemble-time (e.g. "i" constraint).
     * The  outb  %al, %dx  encoding is the only option for all other cases.
     * %1 expands to %dx because  port  is a uint16_t.  %w1 could be used if we had the port number a wider C type */
}

static inline uint8_t inb(uint16_t port)
{
    uint8_t ret;
    asm volatile ( "inb %1, %0"
                   : "=a"(ret)
                   : "Nd"(port) );
    return ret;
}

static inline uint16_t inw(uint16_t port)
{
    uint16_t ret;
    asm volatile ( "inw %1, %0"
                   : "=a"(ret)
                   : "Nd"(port) );
    return ret;
}

static inline void cli() { asm volatile ("cli"); }
static inline void sti() { asm volatile ("sti"); }
static inline void hlt() {
    while(1) {
        asm volatile ("hlt");
    }
}

static inline void io_wait() {
    asm volatile (
        "nop; nop; nop;"
    );
}

static inline void set_cr3(void *m) {
    asm volatile(
        "movl %0, %%cr3"
        :: "r" (m) : "memory"
    );
}

static inline void invlpg(void *m) {
    // Clobber memory to avoid optimizer re-ordering access before invlpg,
    // which may cause nasty bugs.
    asm volatile (
        "invlpg (%0)" :: "b"(m) : "memory"
    );
}
