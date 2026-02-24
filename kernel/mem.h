#ifndef MEM_H
#define MEM_H

#include <stdint.h>

void mem_init(uint64_t start, uint64_t end);
void* kmalloc(uint64_t size);
void kfree(void* ptr); // optional for now

#endif
