#include "memlib.h"
#include "mm.h"
#include "minitest.h"

#include <stdio.h>
#include <string.h>

#define MAXP 128

static void *ptrs[MAXP];
static size_t reqs[MAXP];
static int live[MAXP];

static void paint(void *p, size_t n, unsigned char tag) {
    memset(p, tag, n);
}

static int intact(void *p, size_t n, unsigned char tag) {
    unsigned char *b = p;
    for (size_t i = 0; i < n; i++) {
        if (b[i] != tag) {
            return 0;
        }
    }
    return 1;
}

static int play(const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) {
        perror(path);
        return -1;
    }
    mem_init();
    if (mm_init() != 0) {
        fclose(f);
        return -1;
    }
    memset(ptrs, 0, sizeof(ptrs));
    memset(live, 0, sizeof(live));
    size_t payload = 0;
    size_t peak_payload = 0;
    char line[128];
    int next_id = 0;
    while (fgets(line, sizeof(line), f)) {
        if (line[0] == '#' || line[0] == '\n') {
            continue;
        }
        char op;
        int id = 0;
        unsigned sz = 0;
        if (sscanf(line, " %c %d %u", &op, &id, &sz) < 2 &&
            sscanf(line, " %c %u", &op, &sz) < 1) {
            continue;
        }
        if (op == 'a') {
            sscanf(line, " a %u", &sz);
            id = next_id++;
            void *p = mm_malloc(sz);
            if (!p || mm_check() != 0) {
                fclose(f);
                return -1;
            }
            ptrs[id] = p;
            reqs[id] = sz;
            live[id] = 1;
            paint(p, sz, (unsigned char)(0x40 + id));
            payload += sz;
            if (payload > peak_payload) {
                peak_payload = payload;
            }
        } else if (op == 'f') {
            sscanf(line, " f %d", &id);
            if (!live[id] || !intact(ptrs[id], reqs[id], (unsigned char)(0x40 + id))) {
                fclose(f);
                return -1;
            }
            mm_free(ptrs[id]);
            if (mm_check() != 0) {
                fclose(f);
                return -1;
            }
            payload -= reqs[id];
            live[id] = 0;
            ptrs[id] = NULL;
        } else if (op == 'r') {
            sscanf(line, " r %d %u", &id, &sz);
            if (!live[id] || !intact(ptrs[id], reqs[id], (unsigned char)(0x40 + id))) {
                fclose(f);
                return -1;
            }
            void *p = mm_realloc(ptrs[id], sz);
            if (!p || mm_check() != 0) {
                fclose(f);
                return -1;
            }
            size_t keep = reqs[id] < sz ? reqs[id] : sz;
            if (!intact(p, keep, (unsigned char)(0x40 + id))) {
                fclose(f);
                return -1;
            }
            paint(p, sz, (unsigned char)(0x40 + id));
            payload = payload - reqs[id] + sz;
            if (payload > peak_payload) {
                peak_payload = payload;
            }
            ptrs[id] = p;
            reqs[id] = sz;
        }
    }
    fclose(f);
    size_t heap = mem_heap_size();
    double util = heap ? (double)peak_payload / (double)heap : 0;
    printf("%s  util=%.3f  heap=%zu  peak_payload=%zu\n", path, util, heap, peak_payload);
    if (util < 0.50) {
        fprintf(stderr, "utilization below 0.50\n");
        return -1;
    }
    return 0;
}

int main(void) {
    MT_EQ_INT(play("traces/reuse.trace"), 0);
    MT_EQ_INT(play("traces/realloc.trace"), 0);
    MT_EQ_INT(play("traces/mixed.trace"), 0);
    MT_REPORT();
}
