#include "mm.h"
#include "memlib.h"

#include <stdint.h>
#include <string.h>

/* Implicit free list.
 * Every block is [header][payload][footer], 8-byte aligned.
 * header/footer hold the block size with the low bit set when allocated.
 * An allocated prologue and a size-0 epilogue bracket the list.
 */

#define WORD 4u

static unsigned char *heap0;

static uint32_t rd(const void *p) {
    uint32_t v;
    memcpy(&v, p, 4);
    return v;
}

static void wr(void *p, uint32_t v) { memcpy(p, &v, 4); }

static uint32_t pack(uint32_t size, int alloc) {
    return (size & ~7u) | (alloc ? 1u : 0u);
}

static void set_block(unsigned char *hdr, uint32_t size, int alloc) {
    uint32_t tag = pack(size, alloc);
    wr(hdr, tag);
    wr(hdr + size - WORD, tag);
}

static uint32_t bsize(unsigned char *hdr) { return rd(hdr) & ~7u; }
static int balloc(unsigned char *hdr) { return (int)(rd(hdr) & 1u); }

int mm_init(void) {
    /* Caller initializes the simulated heap with mem_init or mem_init_buf. */
    void *raw = mem_sbrk(16);
    if (raw == (void *)-1) {
        return -1;
    }
    heap0 = raw;
    wr(heap0, 0);
    set_block(heap0 + 4, 8, 1);
    wr(heap0 + 12, pack(0, 1));
    return 0;
}

static unsigned char *coalesce(unsigned char *hdr) {
    uint32_t size = bsize(hdr);
    int prev_alloc = (int)(rd(hdr - WORD) & 1u);
    int next_alloc = balloc(hdr + size);
    if (prev_alloc && next_alloc) {
        return hdr;
    }
    if (prev_alloc && !next_alloc) {
        size += bsize(hdr + size);
        set_block(hdr, size, 0);
        return hdr;
    }
    unsigned char *prev = hdr - bsize(hdr - WORD);
    if (!prev_alloc && next_alloc) {
        size += bsize(prev);
        set_block(prev, size, 0);
        return prev;
    }
    size += bsize(prev) + bsize(hdr + bsize(hdr));
    set_block(prev, size, 0);
    return prev;
}

static unsigned char *extend(uint32_t size) {
    void *bp = mem_sbrk((intptr_t)size);
    if (bp == (void *)-1) {
        return NULL;
    }
    unsigned char *hdr = (unsigned char *)bp - WORD;
    set_block(hdr, size, 0);
    wr(hdr + size, pack(0, 1));
    return coalesce(hdr);
}

static unsigned char *find_fit(uint32_t need) {
    unsigned char *hdr = heap0 + 12;
    for (;;) {
        uint32_t size = bsize(hdr);
        if (size == 0) {
            return NULL;
        }
        if (!balloc(hdr) && size >= need) {
            return hdr;
        }
        hdr += size;
    }
}

static void place(unsigned char *hdr, uint32_t need) {
    uint32_t size = bsize(hdr);
    if (size - need >= 16u) {
        set_block(hdr, need, 1);
        set_block(hdr + need, size - need, 0);
    } else {
        set_block(hdr, size, 1);
    }
}

void *mm_malloc(size_t size) {
    if (size == 0) {
        return NULL;
    }
    uint32_t need = (uint32_t)((size + 8u + 7u) & ~7u);
    if (need < 16u) {
        need = 16u;
    }
    unsigned char *hdr = find_fit(need);
    if (!hdr) {
        hdr = extend(need);
        if (!hdr) {
            return NULL;
        }
    }
    place(hdr, need);
    return hdr + WORD;
}

void mm_free(void *ptr) {
    if (!ptr) {
        return;
    }
    unsigned char *hdr = (unsigned char *)ptr - WORD;
    uint32_t size = bsize(hdr);
    set_block(hdr, size, 0);
    coalesce(hdr);
}

void *mm_realloc(void *ptr, size_t size) {
    if (!ptr) {
        return mm_malloc(size);
    }
    if (size == 0) {
        mm_free(ptr);
        return NULL;
    }
    unsigned char *hdr = (unsigned char *)ptr - WORD;
    uint32_t old = bsize(hdr);
    uint32_t need = (uint32_t)((size + 8u + 7u) & ~7u);
    if (need < 16u) {
        need = 16u;
    }
    if (old >= need) {
        place(hdr, need);
        return ptr;
    }
    unsigned char *next = hdr + old;
    if (!balloc(next) && old + bsize(next) >= need) {
        uint32_t combined = old + bsize(next);
        set_block(hdr, combined, 1);
        place(hdr, need);
        return ptr;
    }
    void *nptr = mm_malloc(size);
    if (!nptr) {
        return NULL;
    }
    size_t copy = old - 8u;
    if (copy > size) {
        copy = size;
    }
    memcpy(nptr, ptr, copy);
    mm_free(ptr);
    return nptr;
}

int mm_check(void) {
    unsigned char *hdr = heap0 + 12;
    int saw_free = 0;
    unsigned char *end = mem_heap_hi();
    while (hdr + WORD <= end) {
        uint32_t size = bsize(hdr);
        if (size == 0) {
            return balloc(hdr) ? 0 : -1;
        }
        if (size < 16 || (size & 7u) || hdr + size > end) {
            return -1;
        }
        if (rd(hdr) != rd(hdr + size - WORD)) {
            return -1;
        }
        if (!balloc(hdr)) {
            if (saw_free) {
                return -1; /* consecutive free blocks should have been merged */
            }
            saw_free = 1;
        } else {
            saw_free = 0;
        }
        hdr += size;
    }
    return -1;
}
