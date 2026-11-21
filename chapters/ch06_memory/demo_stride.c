/* Stride 1 versus stride 8 over the same elements.
 * The stride-8 pass uses eight offsets so the checksum matches.
 */
#include <stdio.h>
#include <stdint.h>
#include <time.h>

enum { N = 1 << 16 };

static volatile uint64_t stride_sink;

static long long elapsed_ns(const struct timespec *t0, const struct timespec *t1) {
    return (t1->tv_sec - t0->tv_sec) * 1000000000LL + (t1->tv_nsec - t0->tv_nsec);
}

static uint64_t sum_stride1(const uint32_t *a) {
    uint64_t sum = 0;
    for (int i = 0; i < N; i++)
        sum += a[i];
    stride_sink = sum;
    return sum;
}

static uint64_t sum_stride8(const uint32_t *a) {
    uint64_t sum = 0;
    for (int off = 0; off < 8; off++) {
        for (int i = off; i < N; i += 8)
            sum += a[i];
    }
    stride_sink = sum;
    return sum;
}

int main(void) {
    static uint32_t a[N];
    for (int i = 0; i < N; i++)
        a[i] = (uint32_t)(i * 3 + 1);

    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);
    uint64_t s1 = sum_stride1(a);
    clock_gettime(CLOCK_MONOTONIC, &t1);
    long long ns1 = elapsed_ns(&t0, &t1);

    clock_gettime(CLOCK_MONOTONIC, &t0);
    uint64_t s8 = sum_stride8(a);
    clock_gettime(CLOCK_MONOTONIC, &t1);
    long long ns8 = elapsed_ns(&t0, &t1);

    printf("stride scan of %d ints (stride 8 uses 8 offsets)\n", N);
    printf("%-12s %20s %16s\n", "pattern", "checksum", "elapsed_ns");
    printf("%-12s %20llu %16lld\n", "stride-1",
           (unsigned long long)s1, ns1);
    printf("%-12s %20llu %16lld\n", "stride-8",
           (unsigned long long)s8, ns8);
    printf("match: %s\n", (s1 == s8) ? "yes" : "no");
    return (s1 == s8) ? 0 : 1;
}
