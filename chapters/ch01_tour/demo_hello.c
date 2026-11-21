/* A tour of one program: source text becomes a process with a stack,
 * a heap, and code mapped from an ELF file. Build steps live in
 * scripts/pipeline.sh.
 */
#include <stdio.h>
#include <stdlib.h>

static int global_data = 40;
static int global_bss;

static int add_one(int x) { return x + 1; }

int main(void) {
    int local = 1;
    int *heap = malloc(sizeof(int));
    if (!heap) {
        perror("malloc");
        return 1;
    }
    *heap = add_one(global_data) + local;

    printf("hello, systems\n");
    printf("code  main        = %p\n", (void *)main);
    printf("data  global_data = %p (%d)\n", (void *)&global_data, global_data);
    printf("bss   global_bss  = %p (%d)\n", (void *)&global_bss, global_bss);
    printf("heap  block       = %p (%d)\n", (void *)heap, *heap);
    printf("stack local       = %p (%d)\n", (void *)&local, local);
    free(heap);
    return 0;
}
