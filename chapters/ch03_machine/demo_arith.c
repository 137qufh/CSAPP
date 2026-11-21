/* Arithmetic that lowers to add, sub, imul, idiv, shl, sar, and shr.
 *
 * A multiply by a small constant is sometimes a lea plus an add:
 *
 *     leaq (%rdi,%rdi,2), %rax     # rax = x*3
 *     addq %rsi, %rax              # rax = x*3 + y
 *
 * Signed right shift (sar) copies the sign bit. Unsigned right shift
 * (shr) fills with zeros. GNU C uses an arithmetic shift for signed
 * operands.
 */
#include <stdio.h>

__attribute__((noinline))
static int mul3_add(int x, int y) {
    return x * 3 + y;
}

__attribute__((noinline))
static int arith_div4(int x) {
    return x / 4;
}

__attribute__((noinline))
static int signed_shr(int x) {
    return x >> 2;
}

__attribute__((noinline))
static unsigned unsigned_shr(unsigned x) {
    return x >> 2;
}

int main(void) {
    printf("20 + 22           = %d\n", 20 + 22);
    printf("50 - 8            = %d\n", 50 - 8);
    printf("6 * 7             = %d\n", 6 * 7);
    printf("mul3_add(10, 12)  = %d\n", mul3_add(10, 12));
    printf("84 / 2            = %d\n", 84 / 2);
    printf("arith_div4(84)    = %d\n", arith_div4(84));
    printf("arith_div4(-5)    = %d\n", arith_div4(-5));
    printf("5 << 3            = %d\n", 5 << 3);
    printf("signed_shr(-16)   = %d\n", signed_shr(-16));
    printf("unsigned_shr(16)  = %u\n", unsigned_shr(16u));
    printf("unsigned_shr high = 0x%x\n", unsigned_shr(0x80000000u));

    if (mul3_add(10, 12) != 42 || arith_div4(84) != 21) {
        return 1;
    }
    if (signed_shr(-16) != -4 || unsigned_shr(16u) != 4u) {
        return 1;
    }
    if (unsigned_shr(0x80000000u) != 0x20000000u) {
        return 1;
    }
    printf("arith checks passed\n");
    return 0;
}
