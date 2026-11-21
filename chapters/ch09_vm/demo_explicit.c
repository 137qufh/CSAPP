#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Explicit free list: each free block stores the address of the next free block
 * in its first word. Allocated blocks are just slices of the arena.
 */

#define ARENA 4096

static unsigned char arena[ARENA] __attribute__((aligned(16)));
static size_t *free_head; /* payload-sized cells of 32 bytes */

#define CELL 32

static void init_arena(void) {
    free_head = (size_t *)arena;
    size_t n = ARENA / CELL;
    for (size_t i = 0; i < n; i++) {
        size_t *cell = (size_t *)(arena + i * CELL);
        size_t *next = (i + 1 < n) ? (size_t *)(arena + (i + 1) * CELL) : NULL;
        *cell = (size_t)next;
    }
}

static void *alloc_cell(void) {
    if (!free_head) {
        return NULL;
    }
    void *p = free_head;
    free_head = (size_t *)(*free_head);
    return p;
}

static void free_cell(void *p) {
    size_t *cell = p;
    *cell = (size_t)free_head;
    free_head = cell;
}

int main(void) {
    init_arena();
    void *a = alloc_cell();
    void *b = alloc_cell();
    if (!a || !b || a == b) {
        return 1;
    }
    memset(a, 0xab, CELL);
    free_cell(a);
    void *c = alloc_cell();
    if (c != a) {
        fprintf(stderr, "expected LIFO reuse\n");
        return 1;
    }
    printf("explicit-ok a=%p b=%p reused=%p\n", a, b, c);
    return 0;
}
