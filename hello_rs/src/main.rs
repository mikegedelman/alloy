#![no_std]

/// A simple application to run in our OS that prints hello
/// This is just for demonstration, linking against our alloy_std
/// library
pub fn main() {
    alloy_std::print("Hello from rust\0");
}
