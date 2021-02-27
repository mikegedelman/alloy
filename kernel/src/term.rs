use core::fmt;
use crate::cpu;


const VGA_WIDTH: isize = 80;
const VGA_HEIGHT: isize = 25;
const VGA_COLOR: u8 = 0x0F;
const VGA_BUFFER: u32 = 0xC00B8000; // virtual address: 0xC0000000 + 0xB8000

// static mut term: TermWriter = TermWriter { row: 0, col: 0 };
use lazy_static::lazy_static;
use spin::Mutex;

lazy_static! {
    pub static ref TERM: Mutex<TermWriter> =
        Mutex::new(TermWriter { row: 0, col: 0 });
}


pub struct TermWriter {
    row: isize,
    col: isize,
}

impl TermWriter {
    fn clear(&mut self) {
        for y in 0..VGA_HEIGHT {
            for x in 0..VGA_WIDTH {
                putcharat(' ', x, y);
            }
        }
        self.row = 0;
        self.col = 0;
    }

    pub fn write(&mut self, s: &str) {
        for c in s.chars() {
            self.putchar(c);
        }
    }

    fn newline(&mut self) {
        if self.row == (VGA_HEIGHT - 1) {
            shift_up();
        } else {
            self.row += 1;
        }
        self.col = 0;
    }

    fn putchar(&mut self, c: char) {
        if c == '\n' || c == '\r' {
            self.newline();
        } else {
            putcharat(c, self.col, self.row);
            self.col +=  1;

            if self.col == VGA_WIDTH {
                self.newline();
            }
        }
    }
}

impl fmt::Write for TermWriter {
    fn write_str(&mut self, s: &str) -> fmt::Result {
        self.write(s);
        Ok(())
    }
}

fn shift_up() {
    for row in 1..VGA_HEIGHT {
        for col in 0..VGA_WIDTH {
            let (c, color) = char_color_at(row, col);
            putu8at_color(c, color, row, col)
        }
    }
}

fn char_color_at(x: isize, y: isize) -> (u8, u8) {
    let term_buffer = VGA_BUFFER as *mut u8;
    let offset = y * 2 * VGA_WIDTH + (x * 2);
    unsafe {
        let c = *term_buffer.offset(offset);
        let color = *term_buffer.offset(offset + 1);
        (c, color)
    }
}

pub fn putcharat(c: char, x: isize, y: isize) {
    let term_buffer = VGA_BUFFER as *mut u8;
    unsafe {
        let offset = y * 2 * VGA_WIDTH + (x * 2);
        *term_buffer.offset(offset) = c as u8;
        *term_buffer.offset(offset + 1) = VGA_COLOR;
    }
}

fn putu8at_color(c: u8, color: u8, x: isize, y: isize) {
    let term_buffer = VGA_BUFFER as *mut u8;
    unsafe {
        let offset = y * 2 * VGA_WIDTH + (x * 2);
        *term_buffer.offset(offset) = c as u8;
        *term_buffer.offset(offset + 1) = color;
    }
}

pub fn init() {
    disable_cursor();
    TERM.lock().clear();
}

pub fn disable_cursor() {
    unsafe {
        cpu::outb(0x3D4, 0x0A);
        cpu::outb(0x3D5, 0x20);
    }
}

/// https://github.com/phil-opp/blog_os/blob/post-03/src/vga_buffer.rs
/// Like the `print!` macro in the standard library, but prints to the VGA text buffer.
#[macro_export]
macro_rules! print {
    ($($arg:tt)*) => ($crate::term::_print(format_args!($($arg)*)));
}

/// Like the `println!` macro in the standard library, but prints to the VGA text buffer.
#[macro_export]
macro_rules! println {
    () => ($crate::print!("\n"));
    ($($arg:tt)*) => ($crate::print!("{}\n", format_args!($($arg)*)));
}

/// Prints the given formatted string to the VGA text buffer through the global `WRITER` instance.
#[doc(hidden)]
pub fn _print(args: fmt::Arguments) {
    use core::fmt::Write;
    TERM.lock().write_fmt(args).unwrap();
}
