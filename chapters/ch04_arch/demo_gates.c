/* Combinational logic built from AND/OR/NOT, the same idea an HCL description
 * would express, written here as ordinary C so it runs without a special tool.
 */
#include <stdio.h>

static int g_not(int a) { return (~a) & 1; }
static int g_and(int a, int b) { return (a & b) & 1; }
static int g_or(int a, int b) { return (a | b) & 1; }
static int g_xor(int a, int b) {
    return g_or(g_and(a, g_not(b)), g_and(g_not(a), b));
}

static int mux2(int s, int a, int b) {
    return g_or(g_and(g_not(s), a), g_and(s, b));
}

static int full_add(int a, int b, int cin, int *cout) {
    int s1 = g_xor(a, b);
    int sum = g_xor(s1, cin);
    *cout = g_or(g_and(a, b), g_and(s1, cin));
    return sum;
}

static int add4(int a, int b) {
    int cin = 0;
    int out = 0;
    for (int i = 0; i < 4; i++) {
        int cout = 0;
        int bit = full_add((a >> i) & 1, (b >> i) & 1, cin, &cout);
        out |= bit << i;
        cin = cout;
    }
    return out;
}

int main(void) {
    if (g_xor(1, 1) != 0 || g_xor(1, 0) != 1) {
        return 1;
    }
    if (mux2(0, 1, 0) != 1 || mux2(1, 1, 0) != 0) {
        return 1;
    }
    for (int a = 0; a < 16; a++) {
        for (int b = 0; b < 16; b++) {
            if (add4(a, b) != ((a + b) & 0xF)) {
                fprintf(stderr, "add4(%d,%d)\n", a, b);
                return 1;
            }
        }
    }
    printf("gates-ok add4(9,6)=%d\n", add4(9, 6));
    return 0;
}
