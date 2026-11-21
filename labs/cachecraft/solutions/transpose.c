#include "transpose.h"

#include <stddef.h>

/* Blocked transpose. Tiles are 8 by 8, which is one 32-byte cache block
 * of ints along a row. n not divisible by 8 falls back to a plain walk.
 */
void transpose(int n, int *src, int *dst) {
    const int tile = 8;
    if (n <= 0)
        return;
    if (n % tile != 0) {
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++)
                dst[(size_t)j * (size_t)n + (size_t)i] =
                    src[(size_t)i * (size_t)n + (size_t)j];
        }
        return;
    }
    for (int ii = 0; ii < n; ii += tile) {
        for (int jj = 0; jj < n; jj += tile) {
            for (int i = ii; i < ii + tile; i++) {
                for (int j = jj; j < jj + tile; j++)
                    dst[(size_t)j * (size_t)n + (size_t)i] =
                        src[(size_t)i * (size_t)n + (size_t)j];
            }
        }
    }
}
