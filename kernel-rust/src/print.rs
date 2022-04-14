use core::fmt;
use lazy_static::lazy_static;
use spin::Mutex;


fn print_raw(s: &str) {
    unsafe {
        for byte in s.bytes() {
            putchar(byte);
            putchar(0);
        }
    }
}

pub struct WriteInterface {}


lazy_static! {
    pub static ref WRITER: Mutex<WriteInterface> = Mutex::new(WriteInterface{});
}

impl fmt::Write for WriteInterface {
    fn write_str(&mut self, s: &str) -> fmt::Result {
        print_raw(s);
        Ok(())
    }
}

#[macro_export]
macro_rules! print {
    ($($arg:tt)*) => {
        $crate::print::_print(format_args!($($arg)*));
    };
}

/// Prints to the host through the serial interface, appending a newline.
#[macro_export]
macro_rules! println {
    () => ($crate::print!("\n"));
    ($fmt:expr) => ($crate::print!(concat!($fmt, "\n")));
    ($fmt:expr, $($arg:tt)*) => ($crate::print!(
        concat!($fmt, "\n"), $($arg)*));
}

#[doc(hidden)]
pub fn _print(args: ::core::fmt::Arguments) {
    use core::fmt::Write;

    WRITER
        .lock()
        .write_fmt(args)
        .expect("Printing to serial failed");
}


extern "C" {
    fn putchar(c: u8);
}

