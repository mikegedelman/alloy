#ifndef _KERNEL_ELF_H
#define _KERNEL_ELF_H

#include <stdint.h>

typedef struct __attribute__((__packed__)) {
    uint32_t magic;
    uint8_t _32_or_64;
    uint8_t endianness;
    uint8_t version;
    uint8_t osabi;
    uint8_t abiversion;
    uint8_t pad[7];
    uint16_t file_type;
    uint16_t isa;
    uint32_t e_version;
    uint32_t e_entry;
    uint32_t e_phoff;
    uint32_t e_shoff;
    uint32_t e_flags;
    uint16_t e_ehsize;
    uint16_t e_phentsize;
    uint16_t e_phnum;
    uint16_t e_shentsize;
    uint16_t e_shnum;
    uint16_t e_shstrndx;
} Elf32;

typedef struct __attribute__((__packed__)) {
    uint32_t p_type;
    uint32_t p_offset;
    uint32_t p_vaddr;
    uint32_t p_paddr;
    uint32_t p_filesz;
    uint32_t p_memsz;
    uint32_t p_flags;
    uint32_t p_align;
} Elf32ProgramHeader;

void exec (void *program);

#endif
