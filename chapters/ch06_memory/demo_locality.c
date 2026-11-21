/* Row-major vs column-major over a 512 by 512 int matrix.
 * Checksums cover the same elements, so they match. Timings may not.
 * This demo always returns 0.
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

enum { N = 512 };

static long long elapsed_ns(const struct timespec *t0, const struct timespec *t1) {
    return (t1->tv_sec - t0->tv_sec) * 1000000000LL + (t1->tv_nsec - t0->tv_nsec);
}

static uint64_t sum_rows(const int *a) {
    uint64_t sum = 0;
    for (int r = 0; r < N; r++) {
        for (int c = 0; c < N; c++)
            sum += (uint32_t)a[(size_t)r * N + (size_t)c];
    }
    return sum;
}

static uint64_t sum_cols(const int *a) {
    uint64_t sum = 0;
    for (int c = 0; c < N; c++) {
        for (int r = 0; r < N; r++)
            sum += (uint32_t)a[(size_t)r * N + (size_t)c];
    }
    return sum;
}

int main(void) {
    int *a = malloc((size_t)N * (size_t)N * sizeof(*a));
    if (!a) {
        fprintf(stderr, "out of memory\n");
        return 0;
    }
    for (int r = 0; r < N; r++) {
        for (int c = 0; c < N; c++)
            a[(size_t)r * N + (size_t)c] = r * 17 + c;
    }

    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);
    uint64_t row = sum_rows(a);
    clock_gettime(CLOCK_MONOTONIC, &t1);
    long long row_ns = elapsed_ns(&t0, &t1);

    clock_gettime(CLOCK_MONOTONIC, &t0);
    uint64_t col = sum_cols(a);
    clock_gettime(CLOCK_MONOTONIC, &t1);
    long long col_ns = elapsed_ns(&t0, &t1);

    printf("locality: %d x %d ints, row-major storage\n", N, N);
    printf("%-12s %20s %16s\n", "traversal", "checksum", "elapsed_ns");
    printf("%-12s %20llu %16lld\n", "row-major",
           (unsigned long long)row, row_ns);
    printf("%-12s %20llu %16lld\n", "col-major",
           (unsigned long long)col, col_ns);
    printf("match: %s\n", (row == col) ? "yes" : "no");

    free(a);
    return 0;
}
