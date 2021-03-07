#[derive(Clone, Copy, Debug)]
#[repr(C, packed(1))]
pub struct Elf32 {
    pub magic: u32,
    pub _32_or_64: u8,
    pub endianness: u8,
    pub version: u8,
    pub osabi: u8,
    pub abiversion: u8,
    pub pad: [u8; 7],
    pub file_type: u16,
    pub isa: u16,
    pub e_version: u32,
    pub e_entry: u32,
    pub e_phoff: u32,
    pub e_shoff: u32,
    pub e_flags: u32,
    pub e_ehsize: u16,
    pub e_phentsize: u16,
    pub e_phnum: u16,
    pub e_shentsize: u16,
    pub e_shnum: u16,
    pub e_shstrndx: u16,
}

#[derive(Clone, Copy, Debug)]
#[repr(C, packed(1))]
pub struct Elf32ProgramHeader {
    pub p_type: u32,
    pub p_offset: u32,
    pub p_vaddr: u32,
    pub p_paddr: u32,
    pub p_filesz: u32,
    pub p_memsz: u32,
    pub p_flags: u32,
    pub p_align: u32,
}
