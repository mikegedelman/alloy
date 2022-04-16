#ifndef _KERNEL_FS_VOLUME_H
#define _KERNEL_FS_VOLUME_H

#include <stdint.h>

typedef struct __attribute__((__packed__)) {
    uint8_t attributes;
    uint8_t chs_partition_start[3];
    uint8_t partition_type;
    uint8_t chs_partition_end[3];
    uint32_t lba_partition_start;
    uint32_t num_sectors;
} PartitionTableEntry;

typedef struct __attribute__((__packed__)) {
    uint8_t bootstrap_code[440];
    uint32_t signature;
    uint16_t reserved;
    PartitionTableEntry partition_table[4];
    uint16_t valid_bootsect_signature;
} MasterBootRecord;

#endif