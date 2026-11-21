#include "cache.h"

#include <stdlib.h>
#include <stdint.h>

typedef struct {
    int valid;
    uint64_t tag;
    unsigned long long stamp;
} line_t;

struct cache {
    int s;
    int E;
    int b;
    int sets;
    unsigned long long clock;
    line_t *lines;
};

cache_t *cache_new(int s, int E, int b) {
    if (s < 0 || b < 0 || E <= 0 || s > 30 || b > 30 || E > 1024)
        return NULL;

    cache_t *c = calloc(1, sizeof(*c));
    if (!c)
        return NULL;
    c->s = s;
    c->E = E;
    c->b = b;
    c->sets = 1 << s;
    c->lines = calloc((size_t)c->sets * (size_t)E, sizeof(*c->lines));
    if (!c->lines) {
        free(c);
        return NULL;
    }
    return c;
}

void cache_free(cache_t *c) {
    if (!c)
        return;
    free(c->lines);
    free(c);
}

static void touch(cache_t *c, uint64_t block, cache_stats_t *st) {
    uint64_t set = block & ((uint64_t)c->sets - 1u);
    uint64_t tag = block >> c->s;
    line_t *row = c->lines + (size_t)set * (size_t)c->E;

    c->clock++;
    for (int i = 0; i < c->E; i++) {
        if (row[i].valid && row[i].tag == tag) {
            row[i].stamp = c->clock;
            st->hits++;
            return;
        }
    }

    st->misses++;
    int victim = -1;
    for (int i = 0; i < c->E; i++) {
        if (!row[i].valid) {
            victim = i;
            break;
        }
    }
    if (victim < 0) {
        victim = 0;
        for (int i = 1; i < c->E; i++) {
            if (row[i].stamp < row[victim].stamp)
                victim = i;
        }
        st->evictions++;
    }
    row[victim].valid = 1;
    row[victim].tag = tag;
    row[victim].stamp = c->clock;
}

void cache_access(cache_t *c, char op, uint64_t addr, int size, cache_stats_t *st) {
    int passes;

    if (!c || !st || size <= 0)
        return;
    if (op == 'I' || op == 'i')
        return;
    if (op == 'M' || op == 'm')
        passes = 2;
    else if (op == 'L' || op == 'l' || op == 'S' || op == 's')
        passes = 1;
    else
        return;

    uint64_t span = (uint64_t)size;
    uint64_t last = (addr > UINT64_MAX - (span - 1)) ? UINT64_MAX : addr + span - 1;
    uint64_t first_block = addr >> c->b;
    uint64_t last_block = last >> c->b;

    for (int pass = 0; pass < passes; pass++) {
        uint64_t block = first_block;
        for (;;) {
            touch(c, block, st);
            if (block == last_block)
                break;
            block++;
        }
    }
}
