/* Driver for the hand-written add_loop in asm/add_loop.s. */
#include <stdio.h>

int add_loop(int n);

static int expect(int n, int want) {
    int got = add_loop(n);
    printf("add_loop(%d) = %d\n", n, got);
    if (got != want) {
        printf("  expected %d\n", want);
        return 1;
    }
    return 0;
}

int main(void) {
    int rc = 0;
    rc |= expect(0, 0);
    rc |= expect(-4, 0);
    rc |= expect(1, 1);
    rc |= expect(5, 15);
    rc |= expect(10, 55);
    rc |= expect(100, 5050);
    if (rc == 0) {
        printf("add_loop checks passed\n");
    }
    return rc;
}
