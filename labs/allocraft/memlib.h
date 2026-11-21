#ifndef ALLOCRAFT_MEMLIB_H
#define ALLOCRAFT_MEMLIB_H

#include <stddef.h>
#include <stdint.h>

/* Simulated heap. The allocator may call only mem_sbrk to grow it. */
void mem_init(void);
void *mem_sbrk(intptr_t incr);
void *mem_heap_lo(void);
void *mem_heap_hi(void);
size_t mem_heap_size(void);

/* Optional: use an external buffer instead of the built-in 1 MiB heap.
 * buf must be 8-byte aligned. Call this instead of mem_init. */
void mem_init_buf(void *buf, size_t n);

#endif
