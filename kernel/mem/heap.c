#include <stdint.h>
#include <kernel/mem/heap.h>
#include <kernel/mem/virtual.h>

static void *heap_begin;
static void *heap_end;
static void *heap_cur;

void heap_init() {
    heap_begin = virtualmem_alloc_page();
    heap_end = heap_begin + ((4 * 1024 * 1024) * 4);
    heap_cur = heap_begin;
}

void *heap_alloc(size_t alloc_sz) {
    if ((heap_begin + alloc_sz) > heap_end) {
        return NULL;
    }

    size_t rounded_alloc_sz = (alloc_sz & 0x1000) + 0x1000;
    void *ret = heap_cur;
    heap_cur += rounded_alloc_sz;

    return ret;
}

void heap_free(void* noop) {
    // Just do nothing lol
}
