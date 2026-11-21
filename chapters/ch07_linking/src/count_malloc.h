#ifndef COUNT_MALLOC_H
#define COUNT_MALLOC_H

/*
 * Compile-time interposition for this translation unit only.
 * A function-like macro rewrites calls written as malloc(...).
 * Parentheses around the real name stop the macro from recursing.
 * Code inside libc is already compiled, so it is not rewritten.
 */
#include <stddef.h>
#include <stdlib.h>

static int sc_mallocs = 0;

static void *sc_raw_malloc(size_t n) {
    return (malloc)(n);
}

#define malloc(n) (sc_mallocs++, sc_raw_malloc(n))

#endif
