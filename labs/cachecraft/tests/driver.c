/* Trace driver. Prints one summary line and nothing else on success.
 *   hits: N  misses: N  evictions: N
 */
#include "cache.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdint.h>

static void usage(const char *argv0) {
    fprintf(stderr, "usage: %s -s <s> -E <E> -b <b> -t <trace>\n", argv0);
}

static int parse_int(const char *text, int *out) {
    char *end = NULL;
    long value = strtol(text, &end, 10);
    if (end == text || *end != '\0' || value > INT_MAX || value < INT_MIN)
        return -1;
    *out = (int)value;
    return 0;
}

/* Return 0 to skip, 1 when op/addr/size are set, -1 on a bad line. */
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

int main(int argc, char **argv) {
    int s = -1;
    int E = -1;
    int b = -1;
    const char *trace = NULL;
    int got_s = 0;
    int got_E = 0;
    int got_b = 0;

    for (int i = 1; i < argc; i++) {
        const char *flag = argv[i];
        if (strcmp(flag, "-s") == 0 || strcmp(flag, "-E") == 0 ||
            strcmp(flag, "-b") == 0 || strcmp(flag, "-t") == 0) {
            if (i + 1 >= argc) {
                usage(argv[0]);
                return 1;
            }
            if (strcmp(flag, "-t") == 0) {
                trace = argv[++i];
            } else {
                int value = 0;
                if (parse_int(argv[++i], &value) != 0) {
                    usage(argv[0]);
                    return 1;
                }
                if (strcmp(flag, "-s") == 0) {
                    s = value;
                    got_s = 1;
                } else if (strcmp(flag, "-E") == 0) {
                    E = value;
                    got_E = 1;
                } else {
                    b = value;
                    got_b = 1;
                }
            }
        } else {
            usage(argv[0]);
            return 1;
        }
    }

    if (!got_s || !got_E || !got_b || !trace) {
        usage(argv[0]);
        return 1;
    }

    cache_t *cache = cache_new(s, E, b);
    if (!cache) {
        fprintf(stderr, "cache_new failed\n");
        return 1;
    }

    FILE *fp = fopen(trace, "r");
    if (!fp) {
        perror(trace);
        cache_free(cache);
        return 1;
    }

    cache_stats_t st = {0, 0, 0};
    char line[256];
    int line_no = 0;
    while (fgets(line, sizeof line, fp)) {
        char op = 0;
        uint64_t addr = 0;
        int size = 0;
        int kind;
        line_no++;
        kind = classify(line, &op, &addr, &size);
        if (kind == 0)
            continue;
        if (kind < 0) {
            fprintf(stderr, "%s:%d: bad trace line\n", trace, line_no);
            fclose(fp);
            cache_free(cache);
            return 1;
        }
        if (op == 'I' || op == 'i')
            continue;
        cache_access(cache, op, addr, size, &st);
    }
    fclose(fp);
    cache_free(cache);

    printf("hits: %lu  misses: %lu  evictions: %lu\n",
           st.hits, st.misses, st.evictions);
    return 0;
}
