//! PS/2 Driver

use lazy_static::lazy_static;
use spin::Mutex;

// Just use the pc_keyboard crate for this for now. TODO: Remove this dependency
// https://docs.rs/pc-keyboard/0.5.0/pc_keyboard/
use pc_keyboard::{layouts, DecodedKey, HandleControl, Keyboard, ScancodeSet1};

// Our KEYBOARD global handles state from multple keypresses, like ShiftDown + c -> C
lazy_static! {
    static ref KEYBOARD: Mutex<Keyboard<layouts::Us104Key, ScancodeSet1>> = Mutex::new(
        Keyboard::new(layouts::Us104Key, ScancodeSet1, HandleControl::Ignore)
    );
}

/// Handle a scancode from the PIC.
/// For now, just print it to the screen.
pub fn process_scancode(scancode: u8) {
    let mut keyboard = KEYBOARD.lock();
    if let Ok(Some(key_event)) = keyboard.add_byte(scancode) {
        if let Some(key) = keyboard.process_keyevent(key_event) {
            match key {
                DecodedKey::Unicode(character) => print!("{}", character),
                DecodedKey::RawKey(key) => print!("{:?}", key),
            }
        }
    }
}
