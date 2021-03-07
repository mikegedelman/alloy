use crate::mem;
use crate::mem::virt::PageDirFlags;
use crate::drivers::ata;
use crate::fs;
use super::elf;
use log::info;

/// This is sloppily implemented, in order to quickly get to the goal of actually exec'ing a binary
pub fn exec(fname: &str) {
    let ata1 = ata::AtaBlockDevice::new(ata::DriveSelect::Master);
    let ustar = fs::ustar::UStarFs::new(ata1);
    let file = ustar.open(fname).unwrap();

    info!("Exec {}... [size: {}]", file.name, file.size);
    let buf = file.read().unwrap();

    unsafe {
        let elf_header = *(buf.as_ptr() as *const elf::Elf32);
        info!("num prog headers {} entsize {}", elf_header.e_phnum, elf_header.e_phentsize);

        // Create a virtual page that contains the start address. This is extremely hacky.
        let phys_addr = mem::physical::alloc_contiguous_4mb().unwrap();
        let virt_addr = elf_header.e_entry & 0xFFC0000;
        mem::virt::VirtualManager::alloc_4mb(virt_addr, phys_addr, PageDirFlags::WRITE | PageDirFlags::PRESENT | PageDirFlags::_4M_PAGE);

        for phnum in 0..elf_header.e_phnum {
            let off = elf_header.e_phoff as isize + (phnum * elf_header.e_phentsize) as isize;
            let prog_header = *(buf.as_ptr().offset(off) as *const elf::Elf32ProgramHeader);
            if prog_header.p_type == 1 {
                info!("off {:#x} file size {:#x}; mem size {:#x}",prog_header.p_offset, prog_header.p_filesz, prog_header.p_memsz);

                // Ensure that the ELF doesn't want to be loaded outside of the virtual memory we set up
                assert!(prog_header.p_vaddr as u32 >= virt_addr && prog_header.p_vaddr < virt_addr + 4*1024*1024);
                info!("loading at {:#x}", prog_header.p_vaddr as u32);
                compiler_builtins::mem::memcpy(prog_header.p_vaddr as *mut u8, buf.as_ptr().offset(prog_header.p_offset as isize), prog_header.p_filesz as usize);
            }
        }

        info!("Jumping into {} @ {:#x}...", fname, elf_header.e_entry);
        // Jump into the program we just loaded
        asm!("jmp {}", in(reg) elf_header.e_entry as u32);
    }
}