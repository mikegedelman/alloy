// We could probably use macros 

pub fn run_tests() {
    trivial_assertion();
}

fn trivial_assertion() {
    print!("trivial assertion... ");
    assert_eq!(1, 0);
    println!("[ok]");
}