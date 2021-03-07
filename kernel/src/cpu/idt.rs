//! Set up the IDT
use seq_macro::seq;

use crate::externs::utils_asm;

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

/// A structure indicating the base address and size of our IDT.
/// This gets passed to the lidt instruction.
static mut IDT_PTR: IdtPtr = IdtPtr { limit: 0, base: 0 };
/// Build a zeroed idt at first - rust complains about initializing statics from other statics otherwise
static mut IDT: [IdtEntry; 256] = seq!(x in 0..256 {
        [
            #(IdtEntry {
                handler_low: 0,
                selector: 0,
                zero: 0,
                type_attr: 0,
                handler_hi: 0,
            },)*
        ]
    });

/// Use the seq! macro to repeat IdtEntry 256 times, once for each
/// extern fn definied in utils_asm.
fn build_idt() -> [IdtEntry; 256] {
    seq!(x in 0..256 {
        [
            #(IdtEntry {
                handler_low: ((utils_asm::int#x as *const u32) as u32 & 0xFFFF) as u16,
                selector: 0x08,
                zero: 0,
                type_attr: 0x8E,
                handler_hi: ((utils_asm::int#x as *const u32) as u32 >> 16) as u16,
            },)*
        ]
    })
}

/// Load the global static variables idt and IDT_PTR,
/// and then instruct the CPU to load the idt from IDT_PTR.
pub unsafe fn init() {
    IDT = build_idt();

    let base_ptr: *const IdtEntry = IDT.as_ptr();
    IDT_PTR = IdtPtr {
        limit: (core::mem::size_of::<IdtEntry>() as u16 * 256) - 1,
        base: base_ptr as u32,
    };

    asm!(
        "lidt [{0}]",
        in(reg) &IDT_PTR,
    );
}
