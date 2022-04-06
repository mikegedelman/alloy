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
}