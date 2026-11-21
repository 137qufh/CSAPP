#include "ex_opt.h"
#include "minitest.h"

static long sum_ref(const int *a, int n) {
    long sum = 0;
    for (int i = 0; i < n; i++)
        sum += a[i];
    return sum;
}

static long dot_ref(const int *a, const int *b, int n) {
    long sum = 0;
    for (int i = 0; i < n; i++)
        sum += (long)a[i] * (long)b[i];
    return sum;
}

static void check_sum(const int *a, int n) {
    long expect = sum_ref(a, n);
    long naive = sum_naive(a, n);
    MT_EQ_INT(naive, expect);
    MT_EQ_INT(sum_unroll2(a, n), naive);
    MT_EQ_INT(sum_dual_accum(a, n), naive);
    MT_EQ_INT(sum_unroll2(a, n), expect);
    MT_EQ_INT(sum_dual_accum(a, n), expect);
}

static void check_dot(const int *a, const int *b, int n) {
    MT_EQ_INT(dot_reassoc(a, b, n), dot_ref(a, b, n));
}

int main(void) {
    int dummy = 0;
    check_sum(&dummy, 0);
    check_dot(&dummy, &dummy, 0);

    int one[] = {42};
    check_sum(one, 1);
    check_dot(one, one, 1);

    int odd[] = {1, 2, 3, 4, 5};
    int odd_b[] = {5, 4, 3, 2, 1};
    check_sum(odd, 5);
    check_dot(odd, odd_b, 5);

    int neg[] = {-8, 3, -1, 4, 0, -6, 9};
    int neg_b[] = {2, -2, 5, 1, 7, -3, 4};
    check_sum(neg, 7);
    check_dot(neg, neg_b, 7);

    int even[] = {10, -10, 10, -10, 1, 1, 1, 1};
    check_sum(even, 8);
    check_dot(even, even, 8);

    /* 100000*100000 does not fit in a 32-bit int. The wide product is 10^10. */
    int big[] = {100000, 100000, -100000};
    int big_b[] = {100000, -100000, 100000};
    check_dot(big, big_b, 3);

    /* (2+3)*(4+5) = 45, while 2*4 + 3*5 = 23. A rewritten product must fail. */
    int p[] = {2, 3};
    int q[] = {4, 5};
    check_dot(p, q, 2);
    MT_EQ_INT(dot_reassoc(p, q, 2), 23);

    int zeros[6] = {0, 0, 0, 0, 0, 0};
    check_sum(zeros, 6);
    check_dot(zeros, even, 6);

    MT_REPORT();
}
