let file = drivers::ata::read_sectors_vec(drivers::ata::DriveSelect::Master, 0, 1).unwrap();

use crate::mem;

pub fn exec_elf(buf: Vec<u8>) { //

    let bin = Elf::parse(&file).unwrap();
    let prog_header = &bin.program_headers[0];

    let phys_addr = mem::physical::alloc_contiguous_4mb().unwrap();

    mem::virt::VirtualManager::alloc_4mb(prog_header.p_vaddr as u32 & 0xFFC00000, phys_addr, PageDirFlags::WRITE | PageDirFlags::PRESENT | PageDirFlags::_4M_PAGE);
    compiler_builtins::mem::memcpy(prog_header.p_vaddr as *mut u8, file.as_ptr(), prog_header.p_filesz as usize);
    asm!("jmp {}", in(reg) bin.header.e_entry as u32);
    println!("program exited");
}