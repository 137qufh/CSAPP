/* A tiny direct-mapped cache, walked on a fixed address list.
 * 4 sets, 1 line each, 4-byte blocks.
 * set = (addr >> 2) & 3, tag = addr >> 4.
 * A miss into an empty line is cold. A miss onto a live line is an eviction.
 */
#include <stdio.h>
#include <stdint.h>

enum { SETS = 4 };

typedef struct {
    int valid;
    uint64_t tag;
} line_t;

static void step(line_t lines[SETS], uint64_t addr,
                 int *hits, int *misses, int *evictions) {
    unsigned set = (unsigned)((addr >> 2) & 3u);
    uint64_t tag = addr >> 4;
    const char *kind;

    if (lines[set].valid && lines[set].tag == tag) {
        kind = "hit";
        (*hits)++;
    } else if (!lines[set].valid) {
        kind = "miss cold";
        (*misses)++;
        lines[set].valid = 1;
        lines[set].tag = tag;
    } else {
        kind = "miss evict";
        (*misses)++;
        (*evictions)++;
        lines[set].tag = tag;
    }

    printf("0x%04llx   %u   %4llu   %s\n",
           (unsigned long long)addr, set, (unsigned long long)tag, kind);
}

int main(void) {
    line_t lines[SETS] = {{0, 0}, {0, 0}, {0, 0}, {0, 0}};
    const uint64_t trace[] = {0x00, 0x04, 0x08, 0x00, 0x10, 0x04, 0x18, 0x10};
    int hits = 0;
    int misses = 0;
    int evictions = 0;

    printf("direct-mapped walk: s=2 E=1 b=2 (4 sets, 4-byte blocks)\n");
    printf("%-8s %3s %6s   %s\n", "addr", "set", "tag", "result");
    for (size_t i = 0; i < sizeof trace / sizeof trace[0]; i++)
        step(lines, trace[i], &hits, &misses, &evictions);

    printf("hits: %d  misses: %d  evictions: %d\n", hits, misses, evictions);
    return 0;
}
