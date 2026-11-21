#include "memsys.h"
#include "cache.h"

#include <stdlib.h>
#include <string.h>

/* Two-level page table over a 64 KiB identity map.
 * Page size is 256 bytes: VPN is 8 bits, split 4/4, offset 8.
 * TLB is 8-entry fully associative LRU. A statistics-only cache sits
 * in front of the backing store (cpu->mem); it does not change values.
 */

#define PAGE_SHIFT 8
#define PAGE_SIZE (1u << PAGE_SHIFT)
#define L1_BITS 4
#define TLB_N 8

typedef struct {
    int valid;
    uint32_t vpn;
    uint32_t ppn;
    unsigned stamp;
} tlb_t;

struct memsys {
    my86_t *cpu;
    cache_t *cache;
    cache_stats_t cache_st;
    tlb_t tlb[TLB_N];
    unsigned stamp;
    uint64_t tlb_hits;
    uint64_t tlb_misses;
    int present[16][16];
    uint32_t ppn[16][16];
};

static int translate(memsys_t *ms, uint32_t va, uint32_t *pa) {
    if (va >= MY86_MEM_SIZE) {
        return -1;
    }
    uint32_t vpn = va >> PAGE_SHIFT;
    uint32_t off = va & (PAGE_SIZE - 1);
    uint32_t l1 = vpn >> L1_BITS;
    uint32_t l2 = vpn & ((1u << L1_BITS) - 1);
    if (!ms->present[l1][l2]) {
        return -1;
    }
    for (int i = 0; i < TLB_N; i++) {
        if (ms->tlb[i].valid && ms->tlb[i].vpn == vpn) {
            ms->tlb[i].stamp = ++ms->stamp;
            ms->tlb_hits++;
            *pa = (ms->tlb[i].ppn << PAGE_SHIFT) | off;
            return 0;
        }
    }
    ms->tlb_misses++;
    int slot = 0;
    for (int i = 1; i < TLB_N; i++) {
        if (!ms->tlb[i].valid) {
            slot = i;
            break;
        }
        if (!ms->tlb[slot].valid) {
            break;
        }
        if (ms->tlb[i].stamp < ms->tlb[slot].stamp) {
            slot = i;
        }
    }
    for (int i = 0; i < TLB_N; i++) {
        if (!ms->tlb[i].valid) {
            slot = i;
            break;
        }
    }
    ms->tlb[slot].valid = 1;
    ms->tlb[slot].vpn = vpn;
    ms->tlb[slot].ppn = ms->ppn[l1][l2];
    ms->tlb[slot].stamp = ++ms->stamp;
    *pa = (ms->tlb[slot].ppn << PAGE_SHIFT) | off;
    return 0;
}

static int access(void *ctx, uint32_t addr, int size, int is_store, uint32_t *data) {
    memsys_t *ms = ctx;
    if (size != 1 && size != 4) {
        return -1;
    }
    uint32_t pa = 0;
    if (translate(ms, addr, &pa) != 0) {
        return -1;
    }
    if (size == 4) {
        uint32_t pa2 = 0;
        if ((addr & 3u) || translate(ms, addr + 3, &pa2) != 0 || (pa >> PAGE_SHIFT) != (pa2 >> PAGE_SHIFT)) {
            return -1;
        }
    }
    cache_access(ms->cache, is_store ? 'S' : 'L', pa, size, &ms->cache_st);
    if (is_store) {
        if (size == 4) {
            ms->cpu->mem[pa] = (uint8_t)(*data);
            ms->cpu->mem[pa + 1] = (uint8_t)(*data >> 8);
            ms->cpu->mem[pa + 2] = (uint8_t)(*data >> 16);
            ms->cpu->mem[pa + 3] = (uint8_t)(*data >> 24);
        } else {
            ms->cpu->mem[pa] = (uint8_t)(*data & 0xffu);
        }
    } else if (size == 4) {
        *data = (uint32_t)ms->cpu->mem[pa] | ((uint32_t)ms->cpu->mem[pa + 1] << 8) |
                ((uint32_t)ms->cpu->mem[pa + 2] << 16) | ((uint32_t)ms->cpu->mem[pa + 3] << 24);
    } else {
        *data = ms->cpu->mem[pa];
    }
    return 0;
}

memsys_t *memsys_new(my86_t *cpu) {
    memsys_t *ms = calloc(1, sizeof(*ms));
    if (!ms) {
        return NULL;
    }
    ms->cpu = cpu;
    ms->cache = cache_new(4, 2, 4); /* 16 sets, 2 ways, 16-byte blocks */
    if (!ms->cache) {
        free(ms);
        return NULL;
    }
    for (uint32_t vpn = 0; vpn < (MY86_MEM_SIZE / PAGE_SIZE); vpn++) {
        uint32_t l1 = vpn >> L1_BITS;
        uint32_t l2 = vpn & ((1u << L1_BITS) - 1);
        ms->present[l1][l2] = 1;
        ms->ppn[l1][l2] = vpn;
    }
    cpu->mem_access = access;
    cpu->mem_ctx = ms;
    return ms;
}

void memsys_free(memsys_t *ms) {
    if (!ms) {
        return;
    }
    cache_free(ms->cache);
    free(ms);
}

void memsys_stats(const memsys_t *ms, unsigned long *hits, unsigned long *misses,
                  unsigned long *evictions, uint64_t *tlb_hits, uint64_t *tlb_misses) {
    *hits = ms->cache_st.hits;
    *misses = ms->cache_st.misses;
    *evictions = ms->cache_st.evictions;
    *tlb_hits = ms->tlb_hits;
    *tlb_misses = ms->tlb_misses;
}
