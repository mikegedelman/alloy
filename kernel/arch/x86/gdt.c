#include <stdint.h>
#include <kernel/arch/x86.h>

typedef struct __attribute__((__packed__)) {
    uint16_t limit;
    uint16_t base_lo;
    uint8_t base_mid;
    uint8_t access;
    uint8_t limit_flags;
    uint8_t base_hi;
} GdtEntry;

typedef struct __attribute__((__packed__)) {
    uint16_t limit;
    uint32_t base;
} GdtPtr;

static GdtEntry gdt[3] = {
    { // null gdt entry
        .limit = 0,
        .base_lo = 0,
        .base_mid = 0,
        .access = 0,
        .limit_flags = 0,
        .base_hi = 0,
    },
    { // code segment
        .limit = 0xFFFF,
        .base_lo = 0,
        .base_mid = 0,
        .access = 0x9A,
        .limit_flags = 0b11001111,
        .base_hi = 0,
    },
    { // data segment
        .limit = 0xFFFF,
        .base_lo = 0,
        .base_mid = 0,
        .access = 0x92,
        .limit_flags = 0b11001111,
        .base_hi = 0,
    },
};

static GdtPtr gdt_ptr;


void init_gdt() {
    gdt_ptr.limit = (sizeof(GdtEntry) * 3) - 1;
    gdt_ptr.base = (uint32_t) gdt;

    __asm__ volatile (
        "lgdt (%0)" ::
        "m" (gdt_ptr)
    );
}

