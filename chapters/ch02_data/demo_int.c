/* Two's complement vs unsigned, extension, truncation, overflow. */
#include <stdio.h>
#include <stdint.h>

static void line(const char *name, unsigned bits, int width) {
    unsigned mask = (width >= 32) ? 0xFFFFFFFFu : (1u << width) - 1u;
    unsigned u = bits & mask;
    unsigned signbit = 1u << (width - 1);
    int t;
    if (width < 32 && (u & signbit)) {
        t = (int)(u | ~mask);
    } else {
        t = (int)u;
    }
    printf("%-16s bits=0x%08X  unsigned=%10u  signed=%11d\n", name, u, u, t);
}

static int trunc_to_short(int x) {
    return (int)(short)x;
}

int main(void) {
    line("zero", 0, 32);
    line("maxint", 0x7FFFFFFF, 32);
    line("minint", 0x80000000u, 32);
    line("neg1", 0xFFFFFFFFu, 32);
    line("byte -1", 0xFF, 8);
    line("byte 127", 0x7F, 8);
    line("byte 128", 0x80, 8);

    int x = 0x123456;
    printf("zero-extend 16: 0x%08X\n", (unsigned)(unsigned short)x);
    printf("sign-extend 16: 0x%08X\n", (unsigned)(short)(unsigned short)0xF234);
    printf("trunc 0x10001 -> short %d\n", trunc_to_short(0x10001));

    int a = 2000000000;
    int b = 2000000000;
    printf("signed add wraps with -fwrapv: %d + %d = %d\n", a, b, a + b);
    printf("unsigned add: %u\n", (unsigned)a + (unsigned)b);
    return 0;
}
