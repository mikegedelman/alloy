#![no_std]
#![feature(asm)]
#![feature(lang_items)]

const SYSCALL_INT: usize = 0x80;

/// Exit the program, returning control to the
/// kernel via an interrupt
/// See interrupts.rs for interrupt conventions
/// Basically: syscall number in eax, first arg in ebx,
/// if necessary
pub fn _exit() {
    unsafe {
        asm!(
            "mov eax, {syscall_num}",
            "int {syscall}",
            syscall_num = const 0x2,
            syscall = const SYSCALL_INT,
        );
    }
}

/// Put the address of s in ebx and then syscall #1 "print"
/// TOOD: A macro for syscalls would be nice
pub fn print(s: &str) {
    unsafe {
        asm!(
            "mov ebx, [{0}]",
            "mov eax, {syscall_num}",
            "int {syscall}",
            in(reg) &s,
            syscall_num = const 0x1,
            syscall = const SYSCALL_INT,
        );
    }
}

/// Due to an outstanding ICE, this function must be generic
/// https://github.com/rust-lang/rust/issues/79559
#[lang = "start"]
pub fn lang_start<T: 'static>(
    main: fn() -> T,
    _argc: isize,
    _argv: *const *const u8,
) -> isize {
    main();
    0
}

/// The below code is *extremely* hacky
/// Basically we're sidestepping having to define crt0 here
/// Define an extern main(), which will point to to the main() that
/// Rust generates for crt0 to call into
extern {
    fn main();
}

/// Define _start as our actual entrypoint, and call into
/// Rust's main(). Rust will do some setup stuff, and then eventually
/// will call into lang_start()
#[no_mangle]
pub extern "C" fn _start() {
    unsafe {
        main();
        _exit();
    }
}

/// We haven't bothered to setup println!, so just print unknown, try to exit,
/// and then loop forever
#[panic_handler]
fn panic(_: &::core::panic::PanicInfo) -> ! {
    print("unknown panic");
    _exit();
    loop {}
}