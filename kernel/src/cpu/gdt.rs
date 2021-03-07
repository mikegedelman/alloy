
#[allow(dead_code)]
#[repr(C, packed(2))]
pub struct GdtEntry {
    limit: u16,
    base_lo: u16,
    base_mid: u8,
    access: u8,
    limit_flags: u8,
    base_hi: u8,
}

/// A pointer to this struct is passed to the lidt instruction.
#[repr(C, packed(2))]
pub struct GdtPtr {
    pub limit: u16,
    pub base: u32,
}

static GDT: [GdtEntry; 3] = [
    GdtEntry { // null gdt entry
        limit: 0,
        base_lo: 0,
        base_mid: 0,
        access: 0,
        limit_flags: 0,
        base_hi: 0,
    },
    GdtEntry { // code segment
        limit: 0xFFFF,
        base_lo: 0,
        base_mid: 0,
        access: 0b10011010,
        limit_flags: 0b11001111,
        base_hi: 0,
    },
    GdtEntry { // data segment
        limit: 0xFFFF,
        base_lo: 0,
        base_mid: 0,
        access: 0b10011010,
        limit_flags: 0b11001111,
        base_hi: 0,
    },
];

static mut GDT_PTR: GdtPtr = GdtPtr { limit: 0, base: 0 };

/// Load our own GDT, because we don't trust what multiboot has set up
/// Assumption: interrupts are disabled when this is called
/// (It's called very early in the boot process)
pub unsafe fn init() {
    let gdt_base_ptr = GDT.as_ptr();
    GDT_PTR = GdtPtr {
        limit: (core::mem::size_of::<GdtEntry>() as u16 * 3) - 1,
        base: gdt_base_ptr as u32,
    };

    // Probably more stuff should happen here to reload the
    // segment registers, but this seems to work for now
    asm!(
        "lgdt [{0}]",
        in(reg) &GDT_PTR,
    );
}
