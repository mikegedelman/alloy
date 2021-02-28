//! For globals defined in the utils.asm file.
use seq_macro::seq;

/// Default representation, alignment lowered to 2.
/// https://rust-lang.github.io/unsafe-code-guidelines/layout/arrays-and-slices.html
/// > If the element type is repr(C) the layout of the array is guaranteed to be the same as
/// > the layout of a C array with the same element type.
#[repr(C, packed(2))]
pub struct IdtEntry {
    pub handler_low: u16,
    pub selector: u16,
    pub zero: u8,
    pub type_attr: u8,
    pub handler_hi: u16,
}

/// A pointer to this struct is passed to the lidt instruction.
#[repr(C, packed(2))]
pub struct IdtPtr {
    pub limit: u16,
    pub base: u32,
}

extern "C" {
    /// static mut is usually a code smell, but we truly only touch these once
    /// and then hand them off to the CPU.
    pub static mut idt: [IdtEntry; 256];
    pub static mut idt_ptr: IdtPtr;

    /// load_gdt loads the GDT defined in utils.asm
    pub fn load_gdt();
    /// load_idt simply calls: lidt [idt_ptr]
    pub fn load_idt();
    /// a helper fn for waiting for io to complete
    pub fn io_wait();

    // references to the functions int[0-255], defined in
    // utils.asm. They simply pass the interrupt number back
    // to our kernel. This is probably dumb, but it works for now.
    seq!(i in 0..256 {
        pub fn int#i();
    });
}
