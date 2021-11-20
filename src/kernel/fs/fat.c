#include <kernel/fs/fat.h>
#include <kernel/drivers/ata.h>
#include <kernel/mem/heap.h>
#include <kernel/stdio.h>
#include <kernel/string.h>
#include <kernel/math.h>

Fat16Fs fat16_init(size_t start_lba) {
    uint8_t buf[512];
    // TODO decouple from ATA driver
    ata_read(&ata1, ATA_MASTER, start_lba, 512, buf);
    FatBootRecord bootrec = *((FatBootRecord*)buf);
    size_t first_fat_offset = start_lba + bootrec.reserved_sectors;
    size_t root_dir_offset = first_fat_offset + (bootrec.sectors_per_fat * bootrec.fats);
    size_t data_offset = root_dir_offset + ((bootrec.max_root_dir_entries * 32) / 512);

    return (Fat16Fs) {
        .start_lba = start_lba,
        .bootrec = bootrec,
        .first_fat_offset = first_fat_offset,
        .root_dir_offset = start_lba + bootrec.reserved_sectors + (bootrec.sectors_per_fat * bootrec.fats),
        .data_offset = data_offset
    };
}

size_t fat16_read_dir(Fat16Fs *fs, FatDirectoryEntry *entries_buf) {
    size_t root_dir_bytes = fs->bootrec.max_root_dir_entries * 32;
    void *root_dir_buf = heap_alloc(root_dir_bytes);
    ata_read(&ata1, ATA_MASTER, fs->root_dir_offset, root_dir_bytes, root_dir_buf);

    size_t entries_buf_pos = 0;
    for (int i = 0; i < fs->bootrec.max_root_dir_entries; i += sizeof(FatDirectoryEntry)) {
        FatDirectoryEntry *entry = (FatDirectoryEntry*)(root_dir_buf + i);
        if (entry->base_name[0] != 0 && entry->base_name[0] != 0xE5) {
            memcpy(entries_buf + entries_buf_pos, entry, sizeof(FatDirectoryEntry));
            entries_buf_pos += 1;
        }
    }

    return entries_buf_pos;
}

// uint16_t get_cluster(uint16_t *raw_fat, uint16_t cluster_num) {
//     uint16_t lo = raw_fat[cluster_num * 2];
//     uint16_t hi = raw_fat[(cluster_num * 2) + 1];
//     return lo | (hi << 8);
// }

/// TODO: this slams 100K of memory every file read by loading the entire FAT... maybe don't
///   do that?
/// TODO: detect loops
size_t get_cluster_chain(Fat16Fs *fs, uint16_t first_cluster, uint16_t *buf) {
    size_t fat_buf_size = fs->bootrec.sectors_per_fat * 512;
    uint16_t *fat_buf = heap_alloc(fat_buf_size);
    ata_read(&ata1, ATA_MASTER, fs->first_fat_offset, fat_buf_size, fat_buf);

    uint16_t next_cluster = first_cluster;
    int cur_buf_pos = 0;
    while (next_cluster < 0xFFF8) {
        buf[cur_buf_pos] = next_cluster;
        next_cluster = fat_buf[next_cluster];
        cur_buf_pos += 1;
    }

    return cur_buf_pos + 1;
}
    
size_t cluster_num_to_lba(Fat16Fs *fs, uint16_t cluster) {
    return fs->data_offset + ((cluster - 2) * fs->bootrec.sectors_per_cluster);
}

// NOTE: dest should be long enough to hold len + 1 chars, for a null terminator.
void fat_render_filename(FatDirectoryEntry *entry, char *buf) {
    int i = 0;
    for (; i < 8; i++) {
        if (entry->base_name[i] == ' ' || entry->base_name[i] == 0) {
            break;
        }
        buf[i] = entry->base_name[i];
    }
    if (entry->ext[0] != ' ' && entry->ext[0] != 0) {
        buf[i] = '.';
        i += 1;
        for (int j = 0; j < 3; j++) {
            buf[i] = entry->ext[j];
            i += 1;
        }
    }
    buf[i] = 0;
}


FatFile *fat_open(Fat16Fs *fs, char *target_filename) {
    FatDirectoryEntry entries[256];
    size_t num_entries = fat16_read_dir(fs, entries);
    for (size_t i = 0; i < num_entries; i++) {
        char filename[12];
        fat_render_filename(&entries[i], filename);

        if (strcmp(target_filename, filename)) {
            FatFile *ret = heap_alloc(sizeof(FatFile));
            ret->fs = *fs;
            ret->first_cluster = entries[i].cluster_lo;
            ret->size = entries[i].file_size;
            memcpy(ret->filename, filename, 12);
            return ret;
        }
    }

    return NULL;
}

size_t fat_read(FatFile *f, uint8_t *buf) {
    size_t bytes_per_cluster = f->fs.bootrec.sectors_per_cluster * 512;
    uint16_t cluster_chain[64];
    size_t num_clusters = get_cluster_chain(&f->fs, f->first_cluster, cluster_chain);

    size_t bytes_read = 0;
    for (size_t i = 0; i < num_clusters; i++) {
        size_t bytes_to_read = MIN(f->size - bytes_read, bytes_per_cluster);
        size_t lba = cluster_num_to_lba(&f->fs, cluster_chain[i]);
        bytes_read += ata_read(&ata1, ATA_MASTER, lba, bytes_to_read, buf + bytes_read);
    }

    return (size_t) bytes_read;
}


// impl<R: BlockRead + Clone> /*Filesystem<R> for*/ Fat16Fs<R> {
//     // pub fn ls(&self, dir: &str) -> Vec<String> {
//     //     let entries = self.read_dir();
//     //     entries.iter().map(|e| e.to_string()).collect()
//     // }

//     pub fn open(&mut self, name: &str) -> FatFile<R> {
//         let entries = self.read_dir();
//         let match_entry = entries.iter().find(|e| e.to_string() == name).unwrap();
    
//         // Need to redefine the file interface. File doesn't have a block reader - files aren't
//         // necessarily contiguous in fat16. Files have a _file reader_
//         // File {
//         //     block_reader: self.reader.clone(),
//         //     lba: 0,
//         //     size: 0,
//         //     name: name.to_string(),
//         // } 
//         FatFile {
//             fs: self.clone(),
//             first_cluster: match_entry.cluster_lo as usize,
//             size: match_entry.file_size as usize,
//             name: match_entry.to_string(),
//         }
//     }
// }

// #[derive(Clone, Debug)]
// pub struct FatFile<R: BlockRead> {
//     pub fs: Fat16Fs<R>,
//     pub first_cluster: usize,
//     pub size: usize,
//     pub name: String,
//     // current_pos
// }

// impl<R: BlockRead + Clone> FatFile<R> {
//     pub fn read(&mut self, buf: &mut [u8]) -> Result<usize, BlockErr> {
//         let clusters = self.fs.get_cluster_chain(self.first_cluster);
//         let bytes_per_cluster = self.fs.bootrec.sectors_per_cluster as usize * 512;

//         let mut bytes_read = 0;
//         for cluster in clusters {
//             let bytes_to_read = core::cmp::min(self.size - bytes_read, bytes_per_cluster);
//             // calc lba from cluster num
//             let lba = self.fs.cluster_num_to_lba(cluster);
//             bytes_read += self.fs.reader.block_read(lba, bytes_to_read, &mut buf[bytes_read..]).unwrap();
//         }

//         Ok(bytes_read)
//     }
// }

