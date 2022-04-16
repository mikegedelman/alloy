#ifndef _KERNEL_FS_FAT_H
#define _KERNEL_FS_FAT_H

#include <stddef.h>
#include <stdint.h>

typedef struct __attribute__((__packed__)) {
    uint8_t jmp_code[3];
    uint8_t oem_identifier[8];
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t fats;
    uint16_t max_root_dir_entries;
    uint16_t total_sectors;
    uint8_t media_descriptor_type;
    uint16_t sectors_per_fat;
    uint16_t sectors_per_track;
    uint16_t heads;
    uint32_t hidden_sectors;
    uint32_t large_sectors;
    uint8_t drive_num;
    uint8_t nt_flags;
    uint8_t signature;
    uint32_t serial_num;
    uint8_t label[11]; // padded with spaces
    uint8_t system_ident[8]; // don't trust this ever
    uint8_t boot_code[448];
    uint16_t bootable_signature;
} FatBootRecord;

typedef struct __attribute__((__packed__)) {
    uint8_t base_name[8];
    uint8_t ext[3];
    uint8_t attributes;
    uint8_t _reserved;
    uint8_t created_at_tenths_of_sec;
    uint16_t created_time;
    uint16_t created_date;
    uint16_t accessed_date;
    uint16_t _cluster_hi; // always 0 for fat16
    uint16_t modified_time;
    uint16_t modified_date;
    uint16_t cluster_lo;
    uint32_t file_size;
} FatDirectoryEntry;

typedef struct __attribute__((__packed__)) {
    size_t start_lba;
    FatBootRecord bootrec;
    size_t first_fat_offset;
    size_t root_dir_offset;
    size_t data_offset;
} Fat16Fs;

typedef struct {
    Fat16Fs fs;
    uint16_t first_cluster;
    size_t size;
    char filename[12];
} FatFile;

Fat16Fs fat16_init(size_t start_lba);
size_t fat16_read_dir(Fat16Fs *fs, FatDirectoryEntry *entries_buf);
FatFile *fat_open(Fat16Fs *fs, char *target_filename);
void fat_render_filename(FatDirectoryEntry *entry, char *buf);
size_t fat_read(FatFile *f, uint8_t *buf);

#endif