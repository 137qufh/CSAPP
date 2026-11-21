#include "ex_locality.h"
#include "minitest.h"

static int ref_sum(const int *a, int rows, int cols) {
    int sum = 0;
    int n = rows * cols;
    for (int i = 0; i < n; i++)
        sum += a[i];
    return sum;
}

static void check(const int *a, int rows, int cols) {
    int expect = ref_sum(a, rows, cols);
    int row = row_sum(a, rows, cols);
    int col = col_sum(a, rows, cols);
    MT_EQ_INT(row, expect);
    MT_EQ_INT(col, expect);
    MT_EQ_INT(row, col);
}

int main(void) {
    int flat[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    check(flat, 3, 4);
    check(flat, 4, 3);
    check(flat, 1, 12);
    check(flat, 12, 1);
    check(flat, 2, 6);

    int neg[] = {-4, 5, -6, 7, 8, -1};
    check(neg, 2, 3);
    check(neg, 3, 2);
    check(neg, 6, 1);

    int one[] = {42};
    check(one, 1, 1);

    int dummy[] = {9, 8, 7};
    check(dummy, 0, 3);
    check(dummy, 4, 0);

    int wide[64];
    for (int i = 0; i < 64; i++)
        wide[i] = (i % 5) - 2;
    check(wide, 8, 8);
    check(wide, 16, 4);
    check(wide, 5, 7);

    MT_REPORT();
}
