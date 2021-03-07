
use alloc::str::from_utf8;
use alloc::string::String;

pub fn strlen(bytes: &[u8]) -> usize {
    match bytes.iter().position(|&x| x == 0) {
        Some(pos) => pos,
        None => bytes.len(),
    }
}

pub fn cstr_slice(bytes: &[u8]) -> &[u8] {
    let len = strlen(&bytes);
    &bytes[0..len]
}

pub fn cstr_to_string(bytes: &[u8]) -> String {
    let str_slice = cstr_slice(bytes);
    String::from(match from_utf8(&str_slice) {
        Ok(x) => x,
        Err(_) => "",
    })
}
