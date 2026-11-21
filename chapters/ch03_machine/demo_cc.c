/* Condition codes.
 *
 * cmp / test subtract or and their operands and throw the result
 * away, leaving ZF, SF, CF, and OF. A later setcc or jcc reads those
 * bits. The C operators <, ==, and > compile to that pair.
 *
 * Below, a small extended asm does the same thing by hand: add, then
 * either setcc or a pushfq that reads the flags. The stack adjustment
 * steps over the 128-byte red zone so the push does not clobber a
 * local the compiler parked there.
 */
#include <stdio.h>

__attribute__((noinline))
static const char *relation(int a, int b) {
    if (a == b) {
        return "eq";
    }
    if (a < b) {
        return "lt";
    }
    return "gt";
}

__attribute__((noinline))
static const char *unsigned_relation(unsigned a, unsigned b) {
    if (a == b) {
        return "eq";
    }
    if (a < b) {
        return "below";
    }
    return "above";
}

/* setz: 1 when the add produced zero. */
__attribute__((noinline))
static int add_was_zero(int x, int y) {
    unsigned char z = 0;
    __asm__ volatile(
        "addl %[y], %[x]\n\t"
        "setz %[z]"
        : [x] "+&r"(x), [z] "=&q"(z)
        : [y] "r"(y)
        : "cc");
    return (int)z;
}

/* RFLAGS after a 32-bit add. Bits: CF 0, ZF 6, SF 7, OF 11. */
__attribute__((noinline))
static unsigned long flags_after_add(int x, int y, int *sum) {
    unsigned long flags = 0;
    int result = x;
    /* Drop below the red zone first. subq itself writes the flags,
     * so the add that we care about has to come after it. */
    __asm__ volatile(
        "subq $128, %%rsp\n\t"
        "addl %[y], %[result]\n\t"
        "pushfq\n\t"
        "popq %[flags]\n\t"
        "addq $128, %%rsp"
        : [result] "+&r"(result), [flags] "=r"(flags)
        : [y] "r"(y)
        : "cc", "memory");
    *sum = result;
    return flags;
}

static int expect_flags(int x, int y) {
    int sum = 0;
    unsigned long flags = flags_after_add(x, y, &sum);
    int zf = (int)((flags >> 6) & 1ul);
    int sf = (int)((flags >> 7) & 1ul);
    int cf = (int)(flags & 1ul);
    int of = (int)((flags >> 11) & 1ul);

    int wrapped = x + y;
    unsigned ux = (unsigned)x;
    unsigned uy = (unsigned)y;
    int want_zf = (wrapped == 0);
    int want_sf = (wrapped < 0);
    int want_cf = (ux + uy) < ux;
    int want_of = (x > 0 && y > 0 && wrapped < 0) || (x < 0 && y < 0 && wrapped >= 0);

    printf("add %11d + %11d = %11d  ZF=%d SF=%d CF=%d OF=%d\n", x, y, sum, zf, sf, cf, of);
    if (sum != wrapped || zf != want_zf || sf != want_sf || cf != want_cf || of != want_of) {
        printf("  flag mismatch\n");
        return 1;
    }
    return 0;
}

int main(void) {
    printf("3 ? 3 -> %s\n", relation(3, 3));
    printf("3 ? 5 -> %s\n", relation(3, 5));
    printf("5 ? 3 -> %s\n", relation(5, 3));
    printf("1u ? 0xffffffffu -> %s\n", unsigned_relation(1u, 0xffffffffu));
    printf("-1 ? 1 (signed) -> %s\n", relation(-1, 1));

    int rc = 0;
    if (add_was_zero(1, -1) != 1 || add_was_zero(2, 2) != 0) {
        printf("setz check failed\n");
        rc = 1;
    }
    rc |= expect_flags(20, 22);
    rc |= expect_flags(0x7fffffff, 1);
    rc |= expect_flags(-1, 1);
    rc |= expect_flags(-5, -5);
    rc |= expect_flags(0, 0);
    if (rc == 0) {
        printf("cc checks passed\n");
    }
    return rc;
}
