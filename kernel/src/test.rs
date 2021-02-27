//! We could probably use macros here to make things fancy but for now we'll
//! just manully copy our tests to run into the run_tests() function.
use crate::cpu::Port;

#[cfg(feature="test")]
#[panic_handler]
fn panic(info:  &::core::panic::PanicInfo) -> ! {
    serial_println!("[failed]\n");
    serial_println!("Error: {}\n", info);
    exit_qemu(QemuExitCode::Failed);
    loop {}
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
#[repr(u8)]
enum QemuExitCode {
    Success = 0x10,
    Failed = 0x11,
}

fn exit_qemu(exit_code: QemuExitCode) {
    let mut port = Port::new(0xf4);
    port.write(exit_code as u8);
}

trait Testable {
    fn run(&self) -> ();
}

impl<T> Testable for T
where
    T: Fn(),
{
    fn run(&self) {
        serial_print!("{}...\t", core::any::type_name::<T>());
        self();
        serial_println!("[ok]");
    }
}

pub fn run_tests() {
    // This can use a vec once we add memory management
    // Until then, bump the array size when adding tests to this
    let tests: [&dyn Testable; 1] = [
        &trivial_assertion
    ];

    for test in &tests {
        test.run();
    }

    exit_qemu(QemuExitCode::Success);
}


fn trivial_assertion() {
    assert_eq!(1, 1);
}
