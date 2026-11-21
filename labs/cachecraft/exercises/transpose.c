#include "transpose.h"

#include <stddef.h>

/* Starter transpose. Elements are correct; dst is written down columns.
 * TODO: when n is a multiple of 8, walk an 8x8 tile at a time so each
 * destination block is reused before it is evicted.
 */
void transpose(int n, int *src, int *dst) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++)
            dst[(size_t)j * (size_t)n + (size_t)i] =
                src[(size_t)i * (size_t)n + (size_t)j];
    }
}
