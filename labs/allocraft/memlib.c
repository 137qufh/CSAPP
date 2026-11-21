#include "memlib.h"

#include <stdint.h>
#include <string.h>

static unsigned char builtin_heap[1 << 20] __attribute__((aligned(8)));
static unsigned char *lo;
static unsigned char *hi;
static unsigned char *brk;
static unsigned char *max;

void mem_init_buf(void *buf, size_t n) {
    lo = buf;
    brk = lo;
    hi = lo;
    max = lo + n;
}

void mem_init(void) {
    mem_init_buf(builtin_heap, sizeof(builtin_heap));
}

void *mem_sbrk(intptr_t incr) {
    if (incr < 0 || brk + incr > max) {
        return (void *)-1;
    }
    unsigned char *old = brk;
    brk += incr;
    hi = brk;
    return old;
}

void *mem_heap_lo(void) { return lo; }
void *mem_heap_hi(void) { return hi; }
size_t mem_heap_size(void) { return (size_t)(hi - lo); }
