#![no_std]
#![no_main]

#[macro_use] extern crate kernel;


#[no_mangle]
extern "C" fn _start() {
    println!("Loading kernel...");
    kernel::proc::exec::exec("kernel");
    println!("Unable to load the kernel... halting");
    loop {}
}
