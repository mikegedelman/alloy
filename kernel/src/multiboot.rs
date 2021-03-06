pub const MAGIC: u32 = 0x2badb002;

#[repr(C, packed(4))]
pub struct MultibootInfo {
    /* Multiboot info version number */
    pub flags: u32,

    /* Available memory from BIOS */
    pub mem_lower: u32,
    pub mem_upper: u32,

    /* "root" partition */
    pub boot_device: u32,

    /* Kernel command line */
    pub cmdline: u32,

    /* Boot-Module list */
    pub mods_count: u32,
    pub mods_addr: u32,

    //   pub syms_i_dont_care_about_rn: [u8; 12],

    //   union
    //   {
    //     multiboot_aout_symbol_table_t aout_sym,
    //     multiboot_elf_section_header_table_t elf_sec,
    //   } u,
    pub tabsize: u32,
    pub strsize: u32,
    pub addr: u32,
    pub reserved: u32,

    /* Memory Mapping buffer */
    pub mmap_length: u32,
    pub mmap_addr: *const MultibootMmapEntry,

    /* Drive Info buffer */
    pub drives_length: u32,
    drives_addr: u32,

    /* ROM configuration table */
    pub config_table: u32,

    /* Boot Loader Name */
    pub boot_loader_name: u32,

    /* APM table */
    pub apm_table: u32,

    /* Video */
    pub vbe_control_info: u32,
    vbe_mode_info: u32,
    vbe_mode: u16,
    vbe_interface_seg: u16,
    vbe_interface_off: u16,
    vbe_interface_len: u16,

    framebuffer_addr: u64,
    pub framebuffer_pitch: u32,
    pub framebuffer_width: u32,
    pub framebuffer_height: u32,
    framebuffer_bpp: u8,
    // #define MULTIBOOT_FRAMEBUFFER_TYPE_INDEXED 0
    // #define MULTIBOOT_FRAMEBUFFER_TYPE_RGB     1
    // #define MULTIBOOT_FRAMEBUFFER_TYPE_EGA_TEXT     2
    framebuffer_type: u8,
    framebuffer_red_field_position: u8,
    framebuffer_red_mask_size: u8,
    framebuffer_green_field_position: u8,
    framebuffer_green_mask_size: u8,
    framebuffer_blue_field_position: u8,
    framebuffer_blue_mask_size: u8,
    //   union
    //   {
    //     // struct
    //     // {
    //     //   framebuffer_palette_addr,
    //     //   framebuffer_palette_num_colors,
    //     // },
    //     struct
    //     {
    //       multiboot_uint8_t framebuffer_red_field_position: u8,
    //       multiboot_uint8_t framebuffer_red_mask_size: u8,
    //       multiboot_uint8_t framebuffer_green_field_position: u8,
    //       multiboot_uint8_t framebuffer_green_mask_size: u8,
    //       multiboot_uint8_t framebuffer_blue_field_position: u8,
    //       multiboot_uint8_t framebuffer_blue_mask_size: u8,
    //     },
    //   },
}

pub struct MultibootMmapEntryIterator {
    cur: *const MultibootMmapEntry,
    end_of_buffer: u32,
}

#[repr(C, packed(4))]
#[derive(Copy, Clone)]
pub struct MultibootMmapEntry {
    pub size: u32,
    pub addr: u64,
    pub len: u64,
    pub type_: u32,
}

impl MultibootInfo {
    pub fn iter_mmap(&self) -> MultibootMmapEntryIterator {
        // can't use .offset() because this value overflows an isize -_-
        let base_addr = self.mmap_addr as u32 + 0xC0000000;

        MultibootMmapEntryIterator {
            cur: base_addr as *const MultibootMmapEntry,
            end_of_buffer: base_addr + self.mmap_length,
        }
    }
}

impl Iterator for MultibootMmapEntryIterator {
    type Item = MultibootMmapEntry;

    fn next(&mut self) -> Option<Self::Item> {
        if (self.cur as u32) >= self.end_of_buffer {
            return None;
        }

        unsafe {
            let ret: MultibootMmapEntry = *self.cur;
            // self.cur = self.cur.add(ret.size as usize); //  as isize);
            self.cur = ((self.cur as u32) + ret.size + 4) as *const MultibootMmapEntry;
            Some(ret.clone())
        }
    }
}

// struct multiboot_mmap_entry
// {
//   multiboot_uint32_t size;
//   multiboot_uint64_t addr;
//   multiboot_uint64_t len;
// #define MULTIBOOT_MEMORY_AVAILABLE              1
// #define MULTIBOOT_MEMORY_RESERVED               2
// #define MULTIBOOT_MEMORY_ACPI_RECLAIMABLE       3
// #define MULTIBOOT_MEMORY_NVS                    4
// #define MULTIBOOT_MEMORY_BADRAM                 5
//   multiboot_uint32_t type;
// } __attribute__((packed));
