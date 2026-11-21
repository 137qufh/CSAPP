#ifndef EX_OPT_H
#define EX_OPT_H

/* Sum a[0..n-1] into a long. n == 0 yields 0. Do not read past n. */
long sum_naive(const int *a, int n);

/* Same sum. Take two elements per trip, then one tail element if n is odd. */
long sum_unroll2(const int *a, int n);

/* Same sum. Keep two running totals and add them once at the end. */
long sum_dual_accum(const int *a, int n);

/* Dot product of a[0..n-1] and b[0..n-1]: Σ a[i]*b[i].
 * Add independent pairs (a[0]*b[0] + a[1]*b[1]) + ...
 * (a[0]+a[1])*(b[0]+b[1]) is a different value. Do not use it.
 * The product itself must be a wide integer, not a 32-bit multiply.
 */
long dot_reassoc(const int *a, const int *b, int n);

#endif
