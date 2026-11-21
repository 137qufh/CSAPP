#include <stdio.h>
#include <stdlib.h>

/*
 * Link-time interposition. The link line passes -Wl,--wrap,malloc,
 * so references to malloc become __wrap_malloc and __real_malloc
 * is the libc allocator. libc's own internal calls are untouched.
 */
extern void *__real_malloc(size_t n);

static int wraps;

void *__wrap_malloc(size_t n) {
    wraps++;
    return __real_malloc(n);
}

int main(void) {
    char *p = malloc(32);
    if (!p) {
        return 1;
    }
    p[0] = 'q';
    if (p[0] != 'q') {
        free(p);
        return 1;
    }
    free(p);
    printf("wraps=%d\n", wraps);
    return wraps >= 1 ? 0 : 1;
}
