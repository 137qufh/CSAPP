#include "ex_forkcount.h"

int pattern_processes(int id) {
    switch (id) {
    case 1:
        /* Each fork doubles the set of processes that reach the next one. */
        return 8;
    case 2:
        /* Parent, the child, and the grandchild the child creates. */
        return 3;
    case 3:
        /* The original loops four times. Each child breaks immediately. */
        return 5;
    default:
        return 0;
    }
}
