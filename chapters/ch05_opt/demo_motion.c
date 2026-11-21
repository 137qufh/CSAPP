/* Sum an array whose length comes from a string.
 * One version calls strlen on every trip; the other hoists it.
 * Both totals are the length, because every element is 1.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static long long elapsed_ns(const struct timespec *t0, const struct timespec *t1) {
    return (t1->tv_sec - t0->tv_sec) * 1000000000LL + (t1->tv_nsec - t0->tv_nsec);
}

static long sum_strlen_each(const int *a, const char *s) {
    long sum = 0;
    for (size_t i = 0; i < strlen(s); i++) {
        sum += a[i];
        /* Keep strlen in the loop. -O1 otherwise proves s is unchanged
         * and calls it once, which erases the code-motion contrast. */
        asm volatile("" ::: "memory");
    }
    return sum;
}

static long sum_strlen_hoisted(const int *a, const char *s) {
    long sum = 0;
    size_t n = strlen(s);
    for (size_t i = 0; i < n; i++)
        sum += a[i];
    return sum;
}

static long long time_call(long (*fn)(const int *, const char *), const int *a,
                           const char *s, long *out) {
    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);
    *out = fn(a, s);
    clock_gettime(CLOCK_MONOTONIC, &t1);
    return elapsed_ns(&t0, &t1);
}

int main(void) {
    const int n = 4000;
    char *s = malloc((size_t)n + 1);
    int *a = malloc((size_t)n * sizeof(*a));
    if (!s || !a) {
        fprintf(stderr, "out of memory\n");
        free(s);
        free(a);
        return 1;
    }
    memset(s, 'x', (size_t)n);
    s[n] = '\0';
    for (int i = 0; i < n; i++)
        a[i] = 1;

    long each = 0;
    long hoist = 0;
    long long ns_each = time_call(sum_strlen_each, a, s, &each);
    long long ns_hoist = time_call(sum_strlen_hoisted, a, s, &hoist);

    printf("code motion: strlen in the condition vs hoisted length\n");
    printf("%-16s %8s %12s %16s\n", "version", "n", "sum", "elapsed_ns");
    printf("%-16s %8d %12ld %16lld\n", "strlen-each", n, each, ns_each);
    printf("%-16s %8d %12ld %16lld\n", "strlen-hoisted", n, hoist, ns_hoist);
    printf("equal: %s\n", (each == hoist) ? "yes" : "no");

    free(s);
    free(a);
    if (each != hoist || each != n)
        return 1;
    return 0;
}
