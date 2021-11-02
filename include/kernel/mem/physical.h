#pragma once

#include <kernel/multiboot.h>

enum AllocStatus {
    ALLOC_SUCCESS,
    ALLOC_NONE_AVAILABLE
};

typedef struct {
    uint32_t addr;
    enum AllocStatus status;
} AllocResult;

size_t first_free_bit(uint8_t);
AllocResult alloc_contiguous_4mb();
void load_multiboot_mmap(MultibootInfo*);
int get_num_free();