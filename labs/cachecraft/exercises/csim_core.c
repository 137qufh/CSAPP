#include "cache.h"

#include <stdlib.h>

/* Student simulator. The starter keeps a dummy object and never counts.
 * Replace cache_access so hits, misses, and evictions follow cache.h.
 */
struct cache {
    int unused;
};

cache_t *cache_new(int s, int E, int b) {
    (void)s;
    (void)E;
    (void)b;
    return calloc(1, sizeof(cache_t));
}

void cache_free(cache_t *c) {
    free(c);
}

void cache_access(cache_t *c, char op, uint64_t addr, int size, cache_stats_t *st) {
    (void)c;
    (void)op;
    (void)addr;
    (void)size;
    (void)st;
    /* TODO: update st->hits, st->misses, and st->evictions. */
}
