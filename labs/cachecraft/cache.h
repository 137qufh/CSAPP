#ifndef CACHECRAFT_CACHE_H
#define CACHECRAFT_CACHE_H

#include <stdint.h>

/* Set-associative cache.
 * s: set-index bits. E: lines in each set. b: block-offset bits.
 * The tag is whatever remains of the low 64 address bits.
 *
 * cache_access accumulates into *st.
 *   'L' and 'S' each touch every block the byte range covers, once.
 *   'M' does a load pass, then a store pass, over that same range.
 *   'I' is ignored. size <= 0 does nothing.
 * A hit refreshes recency. A miss into an invalid line is cold
 * (miss, no eviction). A miss that replaces a live line counts one eviction.
 * Loads and stores use the same rule (write allocate).
 */
typedef struct cache cache_t;

typedef struct {
    unsigned long hits;
    unsigned long misses;
    unsigned long evictions;
} cache_stats_t;

cache_t *cache_new(int s, int E, int b);
void cache_free(cache_t *c);
void cache_access(cache_t *c, char op, uint64_t addr, int size, cache_stats_t *st);

#endif
