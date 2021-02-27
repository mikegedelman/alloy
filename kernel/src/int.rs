// Best interrupts info
// https://alex.dzyoba.com/blog/os-interrupts/



#[no_mangle]
pub unsafe extern "C"
fn isr_handler(x: u32) {
    match x {
        50 => println!("syscall 50"),
        _ => {},
    }
}

#[no_mangle]
pub unsafe extern "C"
fn _exit() {

}