/* Scalar sum versus unroll-by-2 and unroll-by-4. Same integer total. */
#include <stdio.h>

static long sum1(const int *a, int n) {
    long sum = 0;
    for (int i = 0; i < n; i++)
        sum += a[i];
    return sum;
}

static long sum2(const int *a, int n) {
    long sum = 0;
    int i = 0;
    for (; i + 1 < n; i += 2) {
        sum += a[i];
        sum += a[i + 1];
    }
    for (; i < n; i++)
        sum += a[i];
    return sum;
}

static long sum4(const int *a, int n) {
    long sum = 0;
    int i = 0;
    for (; i + 3 < n; i += 4) {
        sum += a[i];
        sum += a[i + 1];
        sum += a[i + 2];
        sum += a[i + 3];
    }
    for (; i < n; i++)
        sum += a[i];
    return sum;
}

static void show(const char *name, int n, long value) {
    printf("%-12s %8d %16ld\n", name, n, value);
}

static int check(const int *a, int n) {
    long s1 = sum1(a, n);
    long s2 = sum2(a, n);
    long s4 = sum4(a, n);
    printf("\n");
    printf("%-12s %8s %16s\n", "kernel", "n", "result");
    show("scalar", n, s1);
    show("unroll-2", n, s2);
    show("unroll-4", n, s4);
    int ok = (s1 == s2 && s2 == s4);
    printf("match: %s\n", ok ? "yes" : "no");
    return ok;
}

int main(void) {
    int odd[] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    int even[] = {4, -3, 2, -1, 8, 6, 5, 1};
    int empty_slot = 0;

    printf("unrolled integer sum\n");
    int ok = 1;
    ok = check(odd, 9) && ok;
    ok = check(even, 8) && ok;
    ok = check(odd, 1) && ok;
    ok = check(&empty_slot, 0) && ok;
    return ok ? 0 : 1;
}
