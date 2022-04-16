#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <kernel/multiboot.h>
#include <kernel/mem/physical.h>
#include <kernel/stdio.h>

#define _4MB 4*1024*1024
#define PHYSICAL_BITMAP_SZ 131072

static uint8_t bitmap[PHYSICAL_BITMAP_SZ];
static int num_free;

/** Calculate the bitmap frame that contains the given memory address. */
size_t frame_num_for_addr(uint32_t addr) {
    return ((addr & 0xFFFFF000) / 0x1000);
}

/** Find the first free bit in the given byte. */
size_t first_free_bit(uint8_t word) {
    uint32_t output;
    uint32_t input = ~(word & 0xFF);
    asm volatile (
        "bsfl %0, %1"
        : "=r" (output)
        : "r" (input)
    );
    
    return (size_t) output;
}

int get_num_free() { return num_free; }

/** Mark the given range of addresses as free in our bitmap. */
void free_range(uint32_t lo_address, uint32_t hi_address) {
    // Determine which frames contain the low and high addresses
    int lo_frame_num = frame_num_for_addr(lo_address);
    int hi_frame_num = frame_num_for_addr(hi_address);

    // Figure out and set bits in the high and low frames to reflect freeness
    int lo_frame_byte = lo_frame_num / 8;
    int lo_frame_bit = lo_frame_num % 8;
    bitmap[lo_frame_byte] = bitmap[lo_frame_byte] & (0xFF << lo_frame_bit);

    int hi_frame_byte = hi_frame_num / 8;
    int hi_frame_bit = hi_frame_num % 8;
    bitmap[hi_frame_byte] = bitmap[hi_frame_byte] & (0xFF >> hi_frame_bit);

    // Set all of the intermediate frames as free
    for (int i = lo_frame_byte + 1; i < hi_frame_byte; i++) {
        bitmap[i] = 0;
    }
    num_free += hi_frame_num - lo_frame_num;
}

/** 
 * Mark the given range of addresses as reserved (non-free) in our bitmap.
 * The opposite of free_range().
 */
void reserve_range(uint32_t lo_address, uint32_t hi_address) {
    uint32_t lo_frame_num = frame_num_for_addr(lo_address);
    uint32_t hi_frame_num = frame_num_for_addr(hi_address);

    uint32_t lo_frame_byte = lo_frame_num / 8;
    uint32_t lo_frame_bit = lo_frame_num % 8;
    bitmap[lo_frame_byte] = bitmap[lo_frame_byte] | (0xFF << lo_frame_bit);

    uint32_t hi_frame_byte = hi_frame_num / 8;
    uint32_t hi_frame_bit = hi_frame_num % 8;
    bitmap[hi_frame_byte] = bitmap[hi_frame_byte] | (0xFF >> hi_frame_bit);

    for (uint32_t i = lo_frame_byte + 1; i < hi_frame_byte; i++) {
        bitmap[i] = 0xFF;
    }
    num_free -= hi_frame_num - lo_frame_num;
}

/** Iterate through the multiboot memory map and set address ranges as free */
void load_multiboot_mmap(MultibootInfo *mb_info) {
    // Initialize the entire bitmap as non-free
    for (int i = 0; i < PHYSICAL_BITMAP_SZ; i++) {
        bitmap[i] = 0xFF;
    }
    num_free = 0;

    
    for (MultibootMmapEntry *cur_entry = mb_info->map_addr; cur_entry->size > 0; cur_entry = (void*) cur_entry + cur_entry->size + 4) {
        // We only care about type == 1
        if (cur_entry->type != 1) {
            continue;
        }
        uint32_t start_addr = cur_entry->addr;
        uint32_t end_addr = cur_entry->addr + cur_entry->len;
        // printf("Found free range: %x - %x\n", start_addr, end_addr);
        free_range(start_addr, end_addr);
    }
}


/**
 * Reserve a frame for use; return its starting address
 * Currently O(n). Not a huge deal but would be a problem if we were in 64-bit space.
 *
pub fn alloc() -> Option<u32> {
    let mut pmgr = PMGR.lock();
    let byte_with_free_frame_idx = match pmgr.bitmap.iter().position(|x| x < &0xFF) {
        Some(x) => x,
        None => {
            return None;
        }
    } as u32;

    let byte_with_free_frame = pmgr.bitmap[byte_with_free_frame_idx as usize];
    let free_bit = first_free_bit(byte_with_free_frame as u8);
    let frame: u32 = byte_with_free_frame_idx * 8 + free_bit as u32;
    pmgr.reserve(frame as usize);
    Some(frame * 0x1000)
}
*/

/**
 * This kernel is lazy and only allocates space 4MB at a time. This is extremely memory
 * inefficient, but it's a lot easier to implement. Since our physical memory manager
 * was written with 4KB granularity, we'll have to do a little math to find a contiuous
 * 4MB chunk
 */
AllocResult alloc_contiguous_4mb() {
    // for _4mb_frame in 0..1023 {
    //     let mut used = false;
    //     for idx in _4mb_frame * 128..(_4mb_frame + 1) * 128 {
    //         if pmgr.bitmap[idx] != 0 {
    //             used = true;
    //             break;
    //         }
    //     }
    //     if !used {
    //         let addr = (_4mb_frame * 1024 * 0x1000) as u32;
    //         info!(
    //             "Reserving physical memory range {:#x} - {:#x}",
    //             addr,
    //             addr + _4MB
    //         );
    //         pmgr.reserve_range(addr, addr + _4MB);
    //         return Some(addr);
    //     }
    // }
    // None

    // Check each 4mb chunk...
    for (int _4mb_frame = 0; _4mb_frame < 1024; _4mb_frame++) {
        bool used = false;

        // ..by looping through each frame in the chunk and checking if it's used
        for (int idx = _4mb_frame * 128; idx < (_4mb_frame + 1) * 128; idx++) {
            if (bitmap[idx] != 0) {
                // We found a used frame, bail out
                used = true;
                break;
            }
        }

        if (!used) {
            uint32_t addr = (_4mb_frame * 1024 * 0x1000);
            // info!(
            //     "Reserving physical memory range {:#x} - {:#x}",
            //     addr,
            //     addr + _4MB
            // );
            // printf("Reserving a phsyical memory range %x - %x\n", addr, addr + _4MB);
            reserve_range(addr, addr + _4MB);
            return (AllocResult) {
                .addr = addr,
                .status = ALLOC_SUCCESS,
            };
        }
    }

    return (AllocResult) {
        .addr = 0x0,
        .status = ALLOC_NONE_AVAILABLE,
    };
}
