#ifndef CAPSTONE_MEMSYS_H
#define CAPSTONE_MEMSYS_H

#include "my86.h"

#include <stdint.h>

typedef struct memsys memsys_t;

memsys_t *memsys_new(my86_t *cpu);
void memsys_free(memsys_t *ms);
void memsys_stats(const memsys_t *ms, unsigned long *hits, unsigned long *misses,
                  unsigned long *evictions, uint64_t *tlb_hits, uint64_t *tlb_misses);

#endif
