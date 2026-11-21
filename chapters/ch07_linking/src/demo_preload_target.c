#include <stdio.h>
#include <stdlib.h>

/* Three direct allocations. libpreload.so counts them under LD_PRELOAD. */
int main(void) {
    void *a = malloc(8);
    void *b = malloc(16);
    void *c = malloc(24);
    if (!a || !b || !c) {
        return 1;
    }
    ((char *)a)[0] = 1;
    ((char *)b)[0] = 2;
    ((char *)c)[0] = 3;
    free(a);
    free(b);
    free(c);
    printf("target-ok\n");
    fflush(stdout);
    return 0;
}
