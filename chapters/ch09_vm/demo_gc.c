#include <stdio.h>
#include <string.h>

/* A mark-sweep collector over a fixed table of objects.
 * Roots are explicit indexes. Unreachable objects are reclaimed.
 */

#define NOBJS 8

typedef struct {
    int live;
    int marked;
    int nptr;
    int ptr[2];
} obj_t;

static obj_t heap[NOBJS];

static void mark(int i) {
    if (i < 0 || i >= NOBJS || !heap[i].live || heap[i].marked) {
        return;
    }
    heap[i].marked = 1;
    for (int k = 0; k < heap[i].nptr; k++) {
        mark(heap[i].ptr[k]);
    }
}

static int sweep(void) {
    int freed = 0;
    for (int i = 0; i < NOBJS; i++) {
        if (heap[i].live && !heap[i].marked) {
            heap[i].live = 0;
            freed++;
        }
        heap[i].marked = 0;
    }
    return freed;
}

int main(void) {
    memset(heap, 0, sizeof(heap));
    for (int i = 0; i < 4; i++) {
        heap[i].live = 1;
    }
    heap[0].nptr = 1;
    heap[0].ptr[0] = 1;
    heap[1].nptr = 1;
    heap[1].ptr[0] = 2;
    /* object 3 is garbage */
    int roots[] = {0};
    mark(roots[0]);
    int freed = sweep();
    printf("freed=%d obj3-live=%d obj2-live=%d\n", freed, heap[3].live, heap[2].live);
    if (freed != 1 || heap[3].live || !heap[2].live) {
        return 1;
    }
    printf("gc-ok\n");
    return 0;
}
