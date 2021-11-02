#pragma once

#include <stdint.h>

#define MULTIBOOT_MAGIC 0x2badb002

typedef struct __attribute__((__packed__)) {
    uint32_t size;
    uint64_t addr;
    uint64_t len;
    uint32_t type;
} MultibootMmapEntry;

typedef struct __attribute__((__packed__)) {
    /* Multiboot info version number */
    uint32_t flags;

    /* Available memory from BIOS */
    uint32_t mem_lower;
    uint32_t mem_upper;

    /* "root" partition */
    uint32_t boot_device;

    /* Kernel command line */
    uint32_t cmdline;

    /* Boot-Module list */
    uint32_t mods_count;
    uint32_t mods_addr;

    //   syms_i_dont_care_about_rn: [u8; 12],

    //   union
    //   {
    //     multiboot_aout_symbol_table_t aout_sym,
    //     multiboot_elf_section_header_table_t elf_sec,
    //   } u,
    uint32_t tabsize;
    uint32_t strsize;
    uint32_t addr;
    uint32_t reserved;

    /* Memory Mapping buffer */
    uint32_t mmap_length;
    MultibootMmapEntry *map_addr;

    /* Drive Info buffer */
    uint32_t drives_length;
    uint32_t drives_addr;

    /* ROM configuration table */
    uint32_t config_table;

    /* Boot Loader Name */
    uint32_t boot_loader_name;

    /* APM table */
    uint32_t apm_table;

    /* Video */
    uint32_t vbe_control_info;
    uint32_t vbe_mode_info;
    uint16_t vbe_mode;
    uint16_t vbe_interface_seg;
    uint16_t vbe_interface_off;
    uint16_t vbe_interface_len;

    uint64_t framebuffer_addr;
    uint32_t framebuffer_pitch;
    uint32_t framebuffer_width;
    uint32_t framebuffer_height;
    uint8_t framebuffer_bp;
    // #define MULTIBOOT_FRAMEBUFFER_TYPE_INDEXED 0
    // #define MULTIBOOT_FRAMEBUFFER_TYPE_RGB     1
    // #define MULTIBOOT_FRAMEBUFFER_TYPE_EGA_TEXT     2
    uint8_t framebuffer_type;
    uint8_t framebuffer_red_field_position;
    uint8_t framebuffer_red_mask_size;
    uint8_t framebuffer_green_field_position;
    uint8_t framebuffer_green_mask_size;
    uint8_t framebuffer_blue_field_position;
    uint8_t framebuffer_blue_mask_size;
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
} MultibootInfo;

