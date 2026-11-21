#include "cache.h"
#include "transpose.h"
#include "minitest.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>
#include <unistd.h>

static int classify(char *line, char *op, uint64_t *addr, int *size) {
    char *p = line;
    char addrbuf[64];
    char *end = NULL;
    unsigned long long value;
    size_t len;

    while (*p == ' ' || *p == '\t')
        p++;
    if (*p == '\0' || *p == '\n' || *p == '#')
        return 0;
    if (sscanf(p, "%c %63[^,],%d", op, addrbuf, size) != 3)
        return -1;

    len = strlen(addrbuf);
    while (len > 0 && (addrbuf[len - 1] == ' ' || addrbuf[len - 1] == '\t'))
        addrbuf[--len] = '\0';

    value = strtoull(addrbuf, &end, 16);
    if (end == addrbuf || *end != '\0')
        return -1;
    *addr = (uint64_t)value;
    return 1;
}

static int join_path(char *dst, size_t dst_len, const char *dir, const char *name) {
    size_t dir_len = strlen(dir);
    size_t name_len = strlen(name);
    if (dir_len + name_len + 1 > dst_len)
        return -1;
    memcpy(dst, dir, dir_len);
    memcpy(dst + dir_len, name, name_len + 1);
    return 0;
}

static FILE *open_trace(const char *filename) {
    char path[PATH_MAX];
    FILE *fp;

    if (join_path(path, sizeof path, "traces/", filename) == 0) {
        fp = fopen(path, "r");
        if (fp)
            return fp;
    }

    char exe[PATH_MAX];
    ssize_t n = readlink("/proc/self/exe", exe, sizeof exe - 1);
    if (n < 0)
        return NULL;
    exe[n] = '\0';
    char *slash = strrchr(exe, '/');
    if (!slash)
        return NULL;
    *slash = '\0';
    if (join_path(path, sizeof path, exe, "/traces/") != 0)
        return NULL;
    size_t used = strlen(path);
    if (join_path(path + used, sizeof path - used, "", filename) != 0)
        return NULL;
    return fopen(path, "r");
}

static int replay(const char *filename, int s, int E, int b, cache_stats_t *st) {
    FILE *fp = open_trace(filename);
    if (!fp) {
        fprintf(stderr, "cannot open trace %s\n", filename);
        return -1;
    }
    cache_t *c = cache_new(s, E, b);
    if (!c) {
        fclose(fp);
        return -1;
    }
    st->hits = 0;
    st->misses = 0;
    st->evictions = 0;

    char line[256];
    int rc = 0;
    while (fgets(line, sizeof line, fp)) {
        char op = 0;
        uint64_t addr = 0;
        int size = 0;
        int kind = classify(line, &op, &addr, &size);
        if (kind == 0)
            continue;
        if (kind < 0) {
            fprintf(stderr, "bad line in %s: %s\n", filename, line);
            rc = -1;
            break;
        }
        if (op == 'I' || op == 'i')
            continue;
        cache_access(c, op, addr, size, st);
    }
    cache_free(c);
    fclose(fp);
    return rc;
}

static void check_trace(const char *filename, int s, int E, int b,
                        unsigned long hits, unsigned long misses,
                        unsigned long evictions) {
    cache_stats_t st;
    MT_ASSERT(replay(filename, s, E, b, &st) == 0);
    MT_EQ_UINT(st.hits, hits);
    MT_EQ_UINT(st.misses, misses);
    MT_EQ_UINT(st.evictions, evictions);
}

static void ref_transpose(int n, const int *src, int *dst) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++)
            dst[(size_t)j * (size_t)n + (size_t)i] =
                src[(size_t)i * (size_t)n + (size_t)j];
    }
}

static void check_transpose(int n) {
    size_t cells = (size_t)n * (size_t)n;
    int *src = malloc(cells * sizeof(*src));
    int *dst = malloc(cells * sizeof(*dst));
    int *exp = malloc(cells * sizeof(*exp));
    MT_ASSERT(src && dst && exp);
    if (!src || !dst || !exp) {
        free(src);
        free(dst);
        free(exp);
        return;
    }
    for (size_t i = 0; i < cells; i++) {
        src[i] = (int)i * 17 - 30;
        dst[i] = 0x111111;
        exp[i] = 0;
    }
    transpose(n, src, dst);
    ref_transpose(n, src, exp);
    int mismatches = 0;
    for (size_t i = 0; i < cells; i++) {
        if (dst[i] != exp[i])
            mismatches++;
    }
    MT_EQ_INT(mismatches, 0);
    free(src);
    free(dst);
    free(exp);
}

/* Address stream for a 32x32 int matrix. Bases are synthetic so the
 * pattern does not depend on malloc alignment.
 * src at 0x100000, dst at 0x200000. Cache: s=5 E=1 b=5.
 */
enum { MAT_N = 32, TILE = 8 };
enum { SRC_BASE = 0x100000, DST_BASE = 0x200000 };

static unsigned long pattern_misses(int blocked) {
    cache_t *c = cache_new(5, 1, 5);
    cache_stats_t st = {0, 0, 0};
    if (!c)
        return 0;

    if (!blocked) {
        for (int i = 0; i < MAT_N; i++) {
            for (int j = 0; j < MAT_N; j++) {
                cache_access(c, 'L',
                             (uint64_t)SRC_BASE + (uint64_t)(i * MAT_N + j) * 4u, 4,
                             &st);
                cache_access(c, 'S',
                             (uint64_t)DST_BASE + (uint64_t)(j * MAT_N + i) * 4u, 4,
                             &st);
            }
        }
    } else {
        for (int ii = 0; ii < MAT_N; ii += TILE) {
            for (int jj = 0; jj < MAT_N; jj += TILE) {
                for (int i = ii; i < ii + TILE; i++) {
                    for (int j = jj; j < jj + TILE; j++) {
                        cache_access(c, 'L',
                                     (uint64_t)SRC_BASE +
                                         (uint64_t)(i * MAT_N + j) * 4u,
                                     4, &st);
                        cache_access(c, 'S',
                                     (uint64_t)DST_BASE +
                                         (uint64_t)(j * MAT_N + i) * 4u,
                                     4, &st);
                    }
                }
            }
        }
    }
    unsigned long misses = st.misses;
    cache_free(c);
    return misses;
}

int main(void) {
    cache_t *c;
    cache_stats_t st;

    /* Cold store, then a second store to the same block hits. */
    c = cache_new(2, 1, 2);
    MT_ASSERT(c != NULL);
    st = (cache_stats_t){0, 0, 0};
    cache_access(c, 'S', 0x0, 4, &st);
    MT_EQ_UINT(st.hits, 0);
    MT_EQ_UINT(st.misses, 1);
    MT_EQ_UINT(st.evictions, 0);
    cache_access(c, 'L', 0x0, 4, &st);
    MT_EQ_UINT(st.hits, 1);
    MT_EQ_UINT(st.misses, 1);
    MT_EQ_UINT(st.evictions, 0);
    cache_free(c);

    /* Modify is a load and then a store. */
    c = cache_new(2, 1, 2);
    st = (cache_stats_t){0, 0, 0};
    cache_access(c, 'M', 0x8, 4, &st);
    MT_EQ_UINT(st.hits, 1);
    MT_EQ_UINT(st.misses, 1);
    MT_EQ_UINT(st.evictions, 0);
    cache_free(c);

    /* Eight bytes, 4-byte blocks: two blocks. Load misses, store hits. */
    c = cache_new(2, 1, 2);
    st = (cache_stats_t){0, 0, 0};
    cache_access(c, 'M', 0x20, 8, &st);
    MT_EQ_UINT(st.hits, 2);
    MT_EQ_UINT(st.misses, 2);
    MT_EQ_UINT(st.evictions, 0);
    cache_free(c);

    /* A load that starts inside a block and stays there is one access. */
    c = cache_new(2, 1, 2);
    st = (cache_stats_t){0, 0, 0};
    cache_access(c, 'L', 0x11, 2, &st);
    MT_EQ_UINT(st.misses, 1);
    MT_EQ_UINT(st.hits, 0);
    cache_access(c, 'L', 0x10, 4, &st);
    MT_EQ_UINT(st.hits, 1);
    cache_free(c);

    /* Same set, different tags, one line: the second access evicts. */
    c = cache_new(2, 1, 2);
    st = (cache_stats_t){0, 0, 0};
    cache_access(c, 'L', 0x0, 4, &st);
    cache_access(c, 'L', 0x10, 4, &st);
    MT_EQ_UINT(st.hits, 0);
    MT_EQ_UINT(st.misses, 2);
    MT_EQ_UINT(st.evictions, 1);
    cache_free(c);

    /* Two ways: a hit refreshes recency, so the other line is the victim. */
    c = cache_new(0, 2, 0);
    st = (cache_stats_t){0, 0, 0};
    cache_access(c, 'L', 1, 1, &st);
    cache_access(c, 'L', 2, 1, &st);
    cache_access(c, 'L', 1, 1, &st);
    cache_access(c, 'L', 3, 1, &st);
    cache_access(c, 'L', 1, 1, &st);
    cache_access(c, 'L', 2, 1, &st);
    MT_EQ_UINT(st.hits, 2);
    MT_EQ_UINT(st.misses, 4);
    MT_EQ_UINT(st.evictions, 2);
    cache_free(c);

    /* Instruction records in the file must not move the counters. */
    check_trace("simple.trace", 2, 1, 2, 4, 10, 7);
    check_trace("simple.trace", 2, 2, 2, 5, 9, 5);
    check_trace("stride.trace", 2, 1, 2, 4, 4, 0);
    check_trace("conflict.trace", 2, 1, 2, 0, 6, 5);
    check_trace("conflict.trace", 2, 2, 2, 1, 5, 3);

    check_transpose(8);
    check_transpose(16);
    check_transpose(32);

    unsigned long naive_m = pattern_misses(0);
    unsigned long blocked_m = pattern_misses(1);
    printf("pattern misses on s=5 E=1 b=5, 32x32 ints: naive=%lu blocked=%lu\n",
           naive_m, blocked_m);
    MT_ASSERT(blocked_m < naive_m);
    MT_ASSERT(blocked_m < 500);
    MT_ASSERT(naive_m > 800);

    MT_REPORT();
}
