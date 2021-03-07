//! For globals defined in the utils.asm file.
use seq_macro::seq;

extern "C" {
    // references to the functions int[0-255], defined in
    // utils.asm. They simply pass the interrupt number back
    // to our kernel. This is probably dumb, but it works for now.
    seq!(i in 0..256 {
        pub fn int#i();
    });
}
