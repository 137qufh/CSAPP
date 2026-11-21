/* Shifts and scaling by powers of two. */
#include <stdio.h>

static int mul_pow2(int x, int k) { return (int)((unsigned)x << k); }

/* Divide toward zero by 2^k, 0 <= k <= 30. */
static int div_pow2_toward0(int x, int k) {
    unsigned sign = (unsigned)x >> 31;
    int bias = (int)sign * ((1 << k) - 1);
    return (x + bias) >> k;
}

int main(void) {
    unsigned u = 0x8000000Cu;
    printf("logical >> 4   0x%08X\n", u >> 4);
    printf("arith   >> 4   0x%08X\n", (unsigned)((int)u >> 4));
    printf("left    << 4   0x%08X\n", u << 4);

    int samples[] = {32, 33, -32, -33, 7, -7};
    for (unsigned i = 0; i < sizeof(samples) / sizeof(samples[0]); i++) {
        int x = samples[i];
        printf("x=%4d  *4=%4d  /4->0=%4d  check=%4d\n", x, mul_pow2(x, 2),
               div_pow2_toward0(x, 2), x / 4);
    }
    return 0;
}
