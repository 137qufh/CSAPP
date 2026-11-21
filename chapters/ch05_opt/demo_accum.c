/* Three integer shapes of one dot product, and three of one polynomial.
 * Pairing independent terms does not change the integer value.
 */
#include <stdio.h>

static long dot_one(const int *a, const int *b, int n) {
    long sum = 0;
    for (int i = 0; i < n; i++)
        sum += (long)a[i] * (long)b[i];
    return sum;
}

static long dot_two(const int *a, const int *b, int n) {
    long s0 = 0;
    long s1 = 0;
    int i = 0;
    for (; i + 1 < n; i += 2) {
        s0 += (long)a[i] * (long)b[i];
        s1 += (long)a[i + 1] * (long)b[i + 1];
    }
    if (i < n)
        s0 += (long)a[i] * (long)b[i];
    return s0 + s1;
}

static long dot_pairs(const int *a, const int *b, int n) {
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

static long poly_horner(const int *c, int n, int x) {
    long acc = 0;
    for (int i = n - 1; i >= 0; i--)
        acc = acc * x + c[i];
    return acc;
}

static long poly_two(const int *c, int n, int x) {
    long s0 = 0;
    long s1 = 0;
    long xp = 1;
    int i = 0;
    for (; i + 1 < n; i += 2) {
        long xodd = xp * x;
        s0 += (long)c[i] * xp;
        s1 += (long)c[i + 1] * xodd;
        xp = xodd * x;
    }
    if (i < n)
        s0 += (long)c[i] * xp;
    return s0 + s1;
}

static long poly_pairs(const int *c, int n, int x) {
    long sum = 0;
    long xp = 1;
    int i = 0;
    for (; i + 1 < n; i += 2) {
        long xodd = xp * x;
        long p0 = (long)c[i] * xp;
        long p1 = (long)c[i + 1] * xodd;
        sum += (p0 + p1);
        xp = xodd * x;
    }
    if (i < n)
        sum += (long)c[i] * xp;
    return sum;
}

static void row(const char *name, long value) {
    printf("%-16s %16ld\n", name, value);
}

int main(void) {
    const int a[] = {1, 3, 5, 7, 9, 2, 4, 6, 8};
    const int b[] = {2, 1, 2, 1, 3, 3, 1, 4, 2};
    const int c[] = {4, 0, 1, 2, 0, 3, 1, 1, 2};
    const int n = 9;
    const int x = 3;

    long d1 = dot_one(a, b, n);
    long d2 = dot_two(a, b, n);
    long d3 = dot_pairs(a, b, n);
    long p1 = poly_horner(c, n, x);
    long p2 = poly_two(c, n, x);
    long p3 = poly_pairs(c, n, x);

    printf("integer kernels, n=%d, polynomial x=%d\n", n, x);
    printf("%-16s %16s\n", "kernel", "result");
    row("dot-one", d1);
    row("dot-two-acc", d2);
    row("dot-reassoc", d3);
    row("poly-horner", p1);
    row("poly-two-acc", p2);
    row("poly-reassoc", p3);

    int ok = (d1 == d2 && d2 == d3 && p1 == p2 && p2 == p3);
    printf("match: %s\n", ok ? "yes" : "no");
    return ok ? 0 : 1;
}
