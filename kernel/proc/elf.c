#include <stddef.h>

#include <kernel/proc/elf.h>
#include <kernel/mem/physical.h>
#include <kernel/mem/virtual.h>
#include <kernel/stdio.h>
#include <kernel/string.h>

 
/// This is sloppily implemented, in order to quickly get to the goal of actually exec'ing a binary
// pub fn exec(buf: &[u8]) {
void exec (void *buf) {
    Elf32 *elf_header = (Elf32*) buf;
    printf("num prog headers %i entsize %i\n", elf_header->e_phnum, elf_header->e_phentsize);

    AllocResult result = alloc_contiguous_4mb();
    uint32_t virt_addr = elf_header->e_entry & 0xFFC0000;
    alloc_4mb(virt_addr, result.addr, PAGEDIR_WRITE | PAGEDIR_PRESENT | PAGEDIR__4M_PAGE);

    for (int i = 0; i < elf_header->e_phnum; i++) {
        int off = (int) elf_header->e_phoff + (int) (i * elf_header->e_phentsize);

        Elf32ProgramHeader *prog_header = (Elf32ProgramHeader*)(buf + off);
        if (prog_header->p_type == 1) {
            printf("off %x file size %x mem size %x\n", prog_header->p_offset, prog_header->p_filesz, prog_header->p_memsz);

             // Ensure that the ELF doesn't want to be loaded outside of the virtual memory we set up
            printf("%x should be in range (%x, %x)\n", prog_header->p_vaddr, virt_addr, virt_addr + 4*1024*1024);
            printf("loading at %x\n", prog_header->p_vaddr);
            memcpy((void*) prog_header->p_vaddr, buf + prog_header->p_offset, prog_header->p_filesz);
        }
    }

    printf("Jumping to %x\n", elf_header->e_entry);
    asm volatile("jmp %0" :: "r" (elf_header->e_entry));

    // // Create a virtual page that contains the start address. This is extremely hacky.
    // let phys_addr = mem::physical::alloc_contiguous_4mb().unwrap();
    // let virt_addr = elf_header.e_entry & 0xFFC0000;
    // mem::virt::VirtualManager::alloc_4mb(virt_addr, phys_addr, PageDirFlags::WRITE | PageDirFlags::PRESENT | PageDirFlags::_4M_PAGE);

    // for phnum in 0..elf_header.e_phnum {
    //     let off = elf_header.e_phoff as isize + (phnum * elf_header.e_phentsize) as isize;
    //     let prog_header = *(buf.as_ptr().offset(off) as *const elf::Elf32ProgramHeader);
    //     if prog_header.p_type == 1 {
    //         info!("off {:#x} file size {:#x}; mem size {:#x}",prog_header.p_offset, prog_header.p_filesz, prog_header.p_memsz);

    //         // Ensure that the ELF doesn't want to be loaded outside of the virtual memory we set up
    //         assert!(prog_header.p_vaddr as u32 >= virt_addr && prog_header.p_vaddr < virt_addr + 4*1024*1024);
    //         info!("loading at {:#x}", prog_header.p_vaddr as u32);
    //         compiler_builtins::mem::memcpy(prog_header.p_vaddr as *mut u8, buf.as_ptr().offset(prog_header.p_offset as isize), prog_header.p_filesz as usize);
    //     }

    //     // info!("Jumping into {} @ {:#x}...", fname, elf_header.e_entry);
    //     // Jump into the program we just loaded
    //     asm!("jmp {}", in(reg) elf_header.e_entry as u32);
    // }
}