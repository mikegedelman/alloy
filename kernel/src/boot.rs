//! Handle early-setup stuff like setting up IDT.
use seq_macro::seq;

use crate::externs::utils_asm;
use crate::externs::utils_asm::{IdtEntry, IdtPtr};

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

/// Load the global static variables idt and idt_ptr,
/// and then instruct the CPU to load the idt from idt_ptr.
pub fn setup_idt() {
    unsafe {
        utils_asm::idt = build_idt();

        let base_ptr: *const IdtEntry = utils_asm::idt.as_ptr();
        utils_asm::idt_ptr = IdtPtr {
            limit: (core::mem::size_of::<IdtEntry>() as u16 * 256) - 1,
            base: base_ptr as u32,
        };

        // Might be nice to be able to call asm! here, but it wants a
        // compile-time constant for lidt, and the address of idt_ptr
        // isn't known until link time. The lidt instruction takes
        // an immediate offset.
        utils_asm::load_idt();
    }
}
