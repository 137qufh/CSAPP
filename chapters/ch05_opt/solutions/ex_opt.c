#include "ex_opt.h"

long sum_naive(const int *a, int n) {
    long sum = 0;
    for (int i = 0; i < n; i++)
        sum += a[i];
    return sum;
}

long sum_unroll2(const int *a, int n) {
    long sum = 0;
    int i = 0;
    for (; i + 1 < n; i += 2) {
        sum += a[i];
        sum += a[i + 1];
    }
    if (i < n)
        sum += a[i];
    return sum;
}

long sum_dual_accum(const int *a, int n) {
    long even = 0;
    long odd = 0;
    int i = 0;
    for (; i + 1 < n; i += 2) {
        even += a[i];
        odd += a[i + 1];
    }
    if (i < n)
        even += a[i];
    return even + odd;
}

long dot_reassoc(const int *a, const int *b, int n) {
    long sum = 0;
    int i = 0;
    for (; i + 1 < n; i += 2) {
        long p0 = (long)a[i] * (long)b[i];
        long p1 = (long)a[i + 1] * (long)b[i + 1];
        sum += (p0 + p1);
    }
    if (i < n)
        sum += (long)a[i] * (long)b[i];
    return sum;
}
