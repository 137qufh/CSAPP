#include "ex_adder.h"

static int g_not(int a) { return (~a) & 1; }
static int g_and(int a, int b) { return (a & b) & 1; }
static int g_or(int a, int b) { return (a | b) & 1; }

int gate_xor(int a, int b) {
    return g_or(g_and(a, g_not(b)), g_and(g_not(a), b));
}

int gate_mux(int sel, int a, int b) {
    return g_or(g_and(g_not(sel), a), g_and(sel & 1, b));
}

int ripple_add4(int a, int b) {
    int cin = 0;
    int out = 0;
    for (int i = 0; i < 4; i++) {
        int aa = (a >> i) & 1;
        int bb = (b >> i) & 1;
        int sum = gate_xor(gate_xor(aa, bb), cin);
        int cout = (g_and(aa, bb)) | (g_and(gate_xor(aa, bb), cin));
        cout &= 1;
        out |= sum << i;
        cin = cout;
    }
    return out;
}
