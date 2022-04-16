#ifndef _KERNEL_DRIVERS_ATA_H
#define _KERNEL_DRIVERS_ATA_H

typedef enum {
    ATA_MASTER = 0xE0,
    ATA_SLAVE = 0xF0,
} DriveSelect;

// TODO incomplete type?
typedef struct {
    uint16_t data;
    uint16_t error;
    uint16_t sectors;
    uint16_t lba_lo;
    uint16_t lba_mid;
    uint16_t lba_hi;
    uint16_t drive_select;
    uint16_t command;
    // TODO: probably save state of last drive select, and skip
    // selecting again if we're reading the same drive, because
    // selecting is slow
} AtaPio;

extern AtaPio ata1;
extern AtaPio ata2;

void ata_init();
int ata_read(AtaPio *ata, DriveSelect drive, size_t lba, int bytes_to_read, uint8_t *dest);

#endif