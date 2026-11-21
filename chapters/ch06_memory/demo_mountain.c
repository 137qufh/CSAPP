/* Short memory mountain.
 * Working sets: 4 KB, 32 KB, 256 KB, 2 MB.
 * Strides are in 4-byte elements: 1, 8, 32.
 * Writes memory_mountain.csv with size_bytes,stride,ns_per_access.
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>

static volatile uint32_t mountain_sink;

static long long elapsed_ns(const struct timespec *t0, const struct timespec *t1) {
    return (t1->tv_sec - t0->tv_sec) * 1000000000LL + (t1->tv_nsec - t0->tv_nsec);
}

static int reps_for(size_t bytes) {
    if (bytes >= (size_t)2 * 1024 * 1024)
        return 4;
    if (bytes >= (size_t)256 * 1024)
        return 20;
    return 100;
}

static double scan(uint32_t *buf, size_t n, int stride, int reps, uint32_t *checksum) {
    volatile uint32_t *view = buf;
    uint32_t acc = 0;
    size_t per = 0;
    for (size_t i = 0; i < n; i += (size_t)stride)
        per++;
    if (per == 0 || reps <= 0)
        return 0.0;

    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);
    for (int r = 0; r < reps; r++) {
        for (size_t i = 0; i < n; i += (size_t)stride)
            acc += view[i];
    }
    clock_gettime(CLOCK_MONOTONIC, &t1);
    mountain_sink = acc;
    *checksum = acc;
    double accesses = (double)per * (double)reps;
    return (double)elapsed_ns(&t0, &t1) / accesses;
}

int main(void) {
    const size_t sizes[] = {
        4 * 1024,
        32 * 1024,
        256 * 1024,
        (size_t)2 * 1024 * 1024
    };
    const int strides[] = {1, 8, 32};
    const char *path = "memory_mountain.csv";

    FILE *fp = fopen(path, "w");
    if (!fp) {
        perror(path);
        return 1;
    }
    fprintf(fp, "size_bytes,stride,ns_per_access\n");

    printf("memory mountain (element = 4 bytes)\n");
    printf("%12s %8s %14s %12s\n", "size_bytes", "stride", "ns_per_access", "checksum");

    for (size_t si = 0; si < sizeof sizes / sizeof sizes[0]; si++) {
        size_t bytes = sizes[si];
        size_t n = bytes / sizeof(uint32_t);
        uint32_t *buf = calloc(n, sizeof(*buf));
        if (!buf) {
            fprintf(stderr, "out of memory\n");
            fclose(fp);
            return 1;
        }
        uint32_t warm = 0;
        for (size_t i = 0; i < n; i++) {
            buf[i] = (uint32_t)(i * 131u + 7u);
            warm += buf[i];
        }
        mountain_sink = warm;

        for (size_t ti = 0; ti < sizeof strides / sizeof strides[0]; ti++) {
            int stride = strides[ti];
            uint32_t checksum = 0;
            double ns = scan(buf, n, stride, reps_for(bytes), &checksum);
            fprintf(fp, "%zu,%d,%.4f\n", bytes, stride, ns);
            printf("%12zu %8d %14.4f %12u\n", bytes, stride, ns, checksum);
        }
        free(buf);
    }
    fclose(fp);

    char cwd[4096];
    if (getcwd(cwd, sizeof cwd))
        printf("csv: %s/%s\n", cwd, path);
    else
        printf("csv: %s\n", path);
    return 0;
}
