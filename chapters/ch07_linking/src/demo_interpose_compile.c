#include "count_malloc.h"

#include <stdio.h>

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
    printf("mallocs=%d\n", sc_mallocs);
    return sc_mallocs >= 1 ? 0 : 1;
}
