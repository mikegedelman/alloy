#include <kernel/all.h>

extern void flush_tss();

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

static GdtEntry gdt[6] = {
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
    { // user code segment
        .limit = 0xFFFF,
        .base_lo = 0,
        .base_mid = 0,
        .access = 0b11111010,
        .limit_flags = 0b11001111,
        .base_hi = 0,
    },
    { // user data segment
        .limit = 0xFFFF,
        .base_lo = 0,
        .base_mid = 0,
        .access = 0b11110010,
        .limit_flags = 0b11001111,
        .base_hi = 0,
    },
    { // tss
        .limit = 0xFFFF,
        .base_lo = 0,
        .base_mid = 0,
        .access = 0b10001001,
        .limit_flags = 0, // I think...
        .base_hi = 0,
    }
 };

static GdtPtr gdt_ptr;

// copied from https://wiki.osdev.org/Getting_to_Ring_3
typedef struct __attribute__((__packed__)) {
    uint32_t prev_tss; // The previous TSS - with hardware task switching these form a kind of backward linked list.
    uint32_t esp0;     // The stack pointer to load when changing to kernel mode.
    uint32_t ss0;      // The stack segment to load when changing to kernel mode.
    // Everything below here is unused.
    uint32_t esp1; // esp and ss 1 and 2 would be used when switching to rings 1 or 2.
    uint32_t ss1;
    uint32_t esp2;
    uint32_t ss2;
    uint32_t cr3;
    uint32_t eip;
    uint32_t eflags;
    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
    uint32_t es;
    uint32_t cs;
    uint32_t ss;
    uint32_t ds;
    uint32_t fs;
    uint32_t gs;
    uint32_t ldt;
    uint16_t trap;
    uint16_t iomap_base;
} TSSEntry;

static TSSEntry tss_entry;

void init_gdt() {
    // Ensure the TSS is initially zero'd.
    memset(&tss_entry, 0, sizeof(tss_entry));
    tss_entry.ss0 = 0x10;

    tss_entry.cs = 0x1b;
    tss_entry.ds = 0x23;
    tss_entry.es = 0x23;
    tss_entry.fs = 0x23;
    tss_entry.gs = 0x23;
    tss_entry.ss = 0x23;

    uint32_t tss_entry_ptr = (uint32_t) &tss_entry;
    uint32_t tss_limit = sizeof(tss_entry) - 1;
    gdt[5].base_lo = tss_entry_ptr & 0xFFFF;
    gdt[5].base_mid = (tss_entry_ptr >> 16) & 0xFF;
    gdt[5].base_hi = tss_entry_ptr >> 24;
    gdt[5].limit = tss_limit & 0xFFFF;
    gdt[5].limit_flags |= tss_limit >> 24;


    gdt_ptr.limit = (sizeof(GdtEntry) * 6) - 1;
    gdt_ptr.base = (uint32_t) gdt;

    __asm__ volatile (
        "lgdt (%0)" ::
        "m" (gdt_ptr)
    );

    flush_tss();
}

void save_kernel_stack(uint32_t stack) {
    tss_entry.esp0 = stack;
}

