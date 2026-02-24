#include "mem.h"

static uint64_t mem_ptr;

void mem_init(uint64_t start, uint64_t end) {
    mem_ptr = start;
    // we could store 'end' for bounds checking later
}

void* kmalloc(uint64_t size) {
    void* ptr = (void*)mem_ptr;
    mem_ptr += size;
    return ptr;
}

void kfree(void* ptr) {
    // optional: not implemented in phase 1
}
