#include <stdint.h>
#include <kernel/proc/exec.h>
#include <kernel/proc/elf.h>
#include <kernel/stdio.h>
#include <kernel/mem/physical.h>
#include <kernel/mem/virtual.h>
#include <kernel/string.h>

void exec(uint8_t *buf) {
    Elf32 *elf_header = (Elf32*) buf;
    printf("num prog headers %d entsize %d\n", elf_header->e_phnum, elf_header->e_phentsize);

    AllocResult alloc_res = alloc_contiguous_4mb();
    if (alloc_res.status != ALLOC_SUCCESS) {
        printf("error allocating memory\n");
        return;
    }
    uint32_t virt_addr = elf_header->e_entry & 0xFFC0000;
    alloc_4mb(virt_addr, alloc_res.addr, PAGEDIR_WRITE | PAGEDIR_PRESENT | PAGEDIR__4M_PAGE);

    for (int phnum = 0; phnum < elf_header->e_phnum; phnum++) {
        int off = elf_header->e_phoff + (phnum * elf_header->e_phentsize);
        Elf32ProgramHeader *prog_header = (Elf32ProgramHeader*)(buf + off);
        if (prog_header->p_type == 1) {
            printf("off %x file size %x; mem size %x\n",prog_header->p_offset, prog_header->p_filesz, prog_header->p_memsz);
            printf("loading at %x\n", prog_header->p_vaddr);
            memcpy((void*)prog_header->p_vaddr, buf + prog_header->p_offset, prog_header->p_filesz);
        }
    }

    printf("Jumping to %x", elf_header ->e_entry);
    asm volatile (
        "jmp %0"
        :: "r" (elf_header->e_entry)
    );
}