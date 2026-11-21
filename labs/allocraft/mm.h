#ifndef ALLOCRAFT_MM_H
#define ALLOCRAFT_MM_H

#include <stddef.h>

int mm_init(void);
void *mm_malloc(size_t size);
void mm_free(void *ptr);
void *mm_realloc(void *ptr, size_t size);
/* Return 0 when the implicit free list is consistent. */
int mm_check(void);

#endif
