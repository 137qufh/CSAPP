#include "ex_locality.h"

#include <stddef.h>

int row_sum(const int *a, int rows, int cols) {
    int sum = 0;
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++)
            sum += a[(size_t)r * (size_t)cols + (size_t)c];
    }
    return sum;
}

int col_sum(const int *a, int rows, int cols) {
    int sum = 0;
    for (int c = 0; c < cols; c++) {
        for (int r = 0; r < rows; r++)
            sum += a[(size_t)r * (size_t)cols + (size_t)c];
    }
    return sum;
}
