/* asmcraft: five-phase reading puzzle.
 *
 * Instructor source. The assigned binary is the stripped `defuse`
 * built from this file. Students should recover the five input lines
 * by reading that binary, not by opening this translation unit.
 *
 * One line per phase on stdin. Success prints "phase N cleared".
 * The first bad line prints "phase N failed" and exits 1. After all
 * five, the program prints "all phases cleared".
 *
 * Phase 1. Twelve bytes stored XOR 0x5A, decoded at run time. The
 *     line must be harbor-light.
 * Phase 2. Six strictly increasing ints whose sum is 100.
 *     One such line is: 1 2 3 4 40 50
 * Phase 3. Function-pointer table, keys 0..4. The line is the key
 *     whose function returns 42, which is key 3.
 * Phase 4. A static linked list holds 4, 8, 15, 16, 23. The line is
 *     those five numbers in order.
 * Phase 5. Length 5, s[0] == 'a', and s[i] == s[i-1] + (i + 1) for
 *     i = 1..4. Read with 1-based positions, that is s[i] == s[i-1] + i
 *     starting from s[1] == 'a'. The only such line is acfjo.
 */
#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NOINLINE __attribute__((noinline))

static void fail(int n) {
    printf("phase %d failed\n", n);
    exit(1);
}

static void clear_phase(int n) {
    printf("phase %d cleared\n", n);
}

static int read_line(char *buf, size_t cap) {
    if (fgets(buf, (int)cap, stdin) == NULL) {
        return -1;
    }
    size_t n = strlen(buf);
    int got_nl = 0;
    if (n > 0 && buf[n - 1] == '\n') {
        buf[--n] = '\0';
        got_nl = 1;
    }
    if (n > 0 && buf[n - 1] == '\r') {
        buf[--n] = '\0';
    }
    if (!got_nl && !feof(stdin)) {
        int ch;
        while ((ch = getchar()) != EOF && ch != '\n') {
        }
        return -1;
    }
    return 0;
}

/* ---- phase 1: masked bytes, key loaded at run time ---- */

enum { P1_LEN = 12 };

static const unsigned char phase1_enc[P1_LEN] = {
    0x32, 0x3b, 0x28, 0x38, 0x35, 0x28, 0x77, 0x36, 0x33, 0x3d, 0x32, 0x2e
};

static volatile unsigned char phase1_key = 0x5A;

NOINLINE static int phase1(const char *line) {
    unsigned char key = phase1_key;
    if (strlen(line) != P1_LEN) {
        return 0;
    }
    for (int i = 0; i < P1_LEN; i++) {
        unsigned char dec = (unsigned char)(phase1_enc[i] ^ key);
        if ((unsigned char)line[i] != dec) {
            return 0;
        }
    }
    return 1;
}

/* ---- phase 2: six increasing integers, sum 100 ---- */

static int parse_ints(const char *s, int *out, int n) {
    const char *p = s;
    for (int i = 0; i < n; i++) {
        char *end = NULL;
        errno = 0;
        long v = strtol(p, &end, 10);
        if (end == p || errno == ERANGE || v > INT_MAX || v < INT_MIN) {
            return 0;
        }
        out[i] = (int)v;
        p = end;
        if (i + 1 < n) {
            if (*p != ' ' && *p != '\t') {
                return 0;
            }
        }
    }
    while (*p == ' ' || *p == '\t') {
        p++;
    }
    return *p == '\0';
}

NOINLINE static int phase2(const char *line) {
    int v[6];
    if (!parse_ints(line, v, 6)) {
        return 0;
    }
    long sum = 0;
    for (int i = 0; i < 6; i++) {
        if (i > 0 && v[i - 1] >= v[i]) {
            return 0;
        }
        sum += v[i];
    }
    return sum == 100;
}

/* ---- phase 3: indirect call through a function table ---- */

NOINLINE static int phase3_fn0(void) {
    volatile int code = 11;
    return code;
}

NOINLINE static int phase3_fn1(void) {
    volatile int code = 23;
    return code;
}

NOINLINE static int phase3_fn2(void) {
    volatile int code = 37;
    return code;
}

NOINLINE static int phase3_fn3(void) {
    volatile int code = 42;
    return code;
}

NOINLINE static int phase3_fn4(void) {
    volatile int code = 58;
    return code;
}

static int (*const phase3_table[5])(void) = {
    phase3_fn0, phase3_fn1, phase3_fn2, phase3_fn3, phase3_fn4
};

NOINLINE static int phase3(const char *line) {
    int key = 0;
    if (!parse_ints(line, &key, 1)) {
        return 0;
    }
    if (key < 0 || key > 4) {
        return 0;
    }
    uintptr_t raw = (uintptr_t)phase3_table[key];
    __asm__ volatile("" : "+r"(raw));
    int (*fn)(void) = (int (*)(void))raw;
    return fn() == 42;
}

/* ---- phase 4: static linked list 4 -> 8 -> 15 -> 16 -> 23 ---- */

struct phase4_link {
    int value;
    struct phase4_link *next;
};

static struct phase4_link phase4_n4 = {23, 0};
static struct phase4_link phase4_n3 = {16, &phase4_n4};
static struct phase4_link phase4_n2 = {15, &phase4_n3};
static struct phase4_link phase4_n1 = {8, &phase4_n2};
static struct phase4_link phase4_n0 = {4, &phase4_n1};
static struct phase4_link *volatile phase4_head = &phase4_n0;

NOINLINE static int phase4_walk(const struct phase4_link *node, const int *want, int n) {
    for (int i = 0; i < n; i++) {
        if (node == NULL || node->value != want[i]) {
            return 0;
        }
        node = node->next;
    }
    return node == NULL;
}

NOINLINE static int phase4(const char *line) {
    int got[5];
    if (!parse_ints(line, got, 5)) {
        return 0;
    }
    const struct phase4_link *head = phase4_head;
    return phase4_walk(head, got, 5);
}

/* ---- phase 5: length 5, 'a' then +2,+3,+4,+5 ---- */

NOINLINE static int phase5(const char *line) {
    const unsigned char *s = (const unsigned char *)line;
    if (s[0] != 'a') {
        return 0;
    }
    for (int i = 1; i < 5; i++) {
        if (s[i] != (unsigned char)(s[i - 1] + (i + 1))) {
            return 0;
        }
    }
    return s[5] == '\0';
}

static int run_phase(int n, int (*check)(const char *)) {
    char line[256];
    if (read_line(line, sizeof line) != 0 || !check(line)) {
        fail(n);
    }
    clear_phase(n);
    return 1;
}

int main(void) {
    run_phase(1, phase1);
    run_phase(2, phase2);
    run_phase(3, phase3);
    run_phase(4, phase4);
    run_phase(5, phase5);
    printf("all phases cleared\n");
    return 0;
}
