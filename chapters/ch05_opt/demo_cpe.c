/* Wall-clock harness. Prints ns per iteration. Not a pass/fail test. */
#include <stdio.h>
#include <time.h>

static volatile long long cpe_sink;

static long long elapsed_ns(const struct timespec *t0, const struct timespec *t1) {
    return (t1->tv_sec - t0->tv_sec) * 1000000000LL + (t1->tv_nsec - t0->tv_nsec);
}

static long long kernel(const int *a, int n, int reps) {
    long long acc = 0;
    for (int r = 0; r < reps; r++) {
        for (int i = 0; i < n; i++)
            acc += a[i];
    }
    cpe_sink = acc;
    return acc;
}

static void measure(const int *a, int n, int reps) {
    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);
    long long sum = kernel(a, n, reps);
    clock_gettime(CLOCK_MONOTONIC, &t1);
    long long ns = elapsed_ns(&t0, &t1);
    long long trips = (long long)reps * (long long)n;
    double per = (trips > 0) ? (double)ns / (double)trips : 0.0;
    printf("%12d %16lld %12.3f %16lld\n", reps, ns, per, sum);
}

int main(void) {
    enum { N = 1024 };
    int a[N];
    for (int i = 0; i < N; i++)
        a[i] = 1;

    printf("loop harness, CLOCK_MONOTONIC, %d-element array of ones\n", N);
    printf("%12s %16s %12s %16s\n", "reps", "elapsed_ns", "ns/iter", "checksum");
    measure(a, N, 200);
    measure(a, N, 800);
    measure(a, N, 3200);
    printf("ns/iter is wall time per addition, not a hardware cycle count.\n");
    return 0;
}
