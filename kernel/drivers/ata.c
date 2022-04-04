/* ATA PIO driver
 * It's dumb and slow, but it's enough to load stuff from disk
 */
#include <stddef.h>
#include <stdint.h>
#include <kernel/stdio.h>
#include <kernel/cpu.h>
#include <kernel/drivers/ata.h>
#include <kernel/string.h>
#include <kernel/math.h>

#define COMMAND_READ 0x20
#define COMMAND_IDENTIFY 0xEC

#define STATUS_ERR 1
#define STATUS_DRQ 1 << 3
#define STATUS_DRF 1 << 5
#define STATUS_BSY 1 << 7

#define ATA1_BASE 0x1F0
#define ATA2_BASE 0x170


AtaPio ata1;
AtaPio ata2;

typedef enum {
    ATA_INVALID_DRIVE = -1,
    ATA_READ_ERROR = -2,
    ATA_DRIVE_FAULT_ERROR = -3
} AtaErr;


void ata_init() {
    ata1.data = ATA1_BASE;
    ata1.error = ATA1_BASE + 1;
    ata1.sectors = ATA1_BASE + 2;
    ata1.lba_lo = ATA1_BASE + 3;
    ata1.lba_mid = ATA1_BASE + 4;
    ata1.lba_hi = ATA1_BASE + 5;
    ata1.drive_select = ATA1_BASE + 6;
    ata1.command = ATA1_BASE + 7;

    ata2.data = ATA1_BASE;
    ata2.error = ATA1_BASE + 1;
    ata2.sectors = ATA1_BASE + 2;
    ata2.lba_lo = ATA1_BASE + 3;
    ata2.lba_mid = ATA1_BASE + 4;
    ata2.lba_hi = ATA1_BASE + 5;
    ata2.drive_select = ATA1_BASE + 6;
    ata2.command = ATA1_BASE + 7;
}


uint8_t read_command(AtaPio *ata, DriveSelect drive, uint32_t lba, uint8_t sectors) {
    outb(ata->drive_select, drive | ((lba >> 24) & 0xF));
    outb(ata->sectors, sectors);
    outb(ata->lba_lo, lba & 0xFF);
    outb(ata->lba_mid, (lba >> 8) & 0xFF);
    outb(ata->lba_hi, (lba >> 16) & 0xFF);
    outb(ata->command, COMMAND_READ);

    // 400ns delay. See: https://wiki.osdev.org/ATA_PIO_Mode#400ns_delays
    for (int i = 0; i < 14; i++) {
        inb(ata->command);
    }

    return inb(ata->command);
}

void ata_wait_bsy(AtaPio *ata) {
    while ((inb(ata->command) & STATUS_BSY) != 0) {}
}

void ata_wait_drq(AtaPio *ata) {
    while ((inb(ata->command) & STATUS_DRQ) == 0) {}
}

// pub fn identify(drive: DriveSelect) -> Result<(), BlockErr> {
//     disable_int();
//     let mut ata = ATA1;
//     ata.identify_command(drive).unwrap();
//     enable_int();
//     Ok(())
// }

int read_bytes(AtaPio *ata, DriveSelect drive, size_t lba, size_t bytes_to_read, uint8_t *buf) {
    // assert bytes_to_read < 512 * 255 ?
    cli();
    ata_wait_bsy(ata);
    int sectors = ((bytes_to_read + 511) / 512);
    int status = read_command(ata, drive, lba, sectors);
    int bytes_read = 0;

    if (status == 0) {
        return ATA_INVALID_DRIVE;
    } else if ((status & 1) == 1) {
        return ATA_READ_ERROR;
    } else if ((status & STATUS_DRF) != 0) {
        return ATA_DRIVE_FAULT_ERROR;
    }

    for (int sector = 0; sector < sectors; sector++) {
        ata_wait_bsy(ata);
        ata_wait_drq(ata);

        uint16_t tmp_buf[256];
        for (int i = 0; i < 256; i++) {
            tmp_buf[i] = inw(ata->data);
        }

        int bytes_to_copy = MIN(bytes_to_read - bytes_read, 512);
        memcpy(buf + (sector * 512), tmp_buf, bytes_to_copy);
        bytes_read += bytes_to_copy;
    }

    sti();
    return bytes_read;
}

int ata_read(AtaPio *ata, DriveSelect drive, size_t lba, int bytes_to_read, uint8_t *buf) {
    size_t bytes_read = 0;

    while (bytes_read < bytes_to_read) {
        int bytes_left = bytes_to_read - bytes_read;
        int bytes_to_read_this_loop = MIN(bytes_left, 255 * 512);

        bytes_read += read_bytes(
            ata,
            drive,
            lba + (bytes_read / 512),
            bytes_to_read_this_loop,
            buf + bytes_read
        );
    }

    return bytes_read;
}