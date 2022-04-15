#![no_std]

mod print;

use core::fmt;

#[repr(C)]
struct NetInfo {
    mac_address: [u8; 6],
}

extern "C" {
    fn get_mac_address() -> NetInfo;
    fn send_packet(data: *const u8, len: u32);
}


#[no_mangle]
pub extern "C" fn rust_main() {
    
    let info = unsafe { get_mac_address() };
    for i in 0..6 {
        print!("{:x} ", info.mac_address[i]);
    }

    let mut buf: [u8; 60] = [0u8; 60];
    for i in 0..6 {
        buf[i] = info.mac_address[i];    
    }

    unsafe {
        send_packet(buf.as_ptr() as *const u8, 60);
    }

}


// Invoked when something calls panic!
#[cfg(not(feature = "test"))]
#[panic_handler]
fn panic(info: &::core::panic::PanicInfo) -> ! {
    println!("panic from rust\n{:?}", info);
    loop {}
}
