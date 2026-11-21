/* A dense switch. At -O1 and -Og, GCC turns the case labels 0..7
 * into a jump table: range-check the key, then an indirect jump
 * through a table of code addresses. The bodies are different
 * operations so the compiler cannot replace the switch with one
 * arithmetic formula.
 */
#include <stdio.h>

__attribute__((noinline))
static int dispatch(int key, int x) {
    switch (key) {
    case 0:
        return x + 1;
    case 1:
        return x - 1;
    case 2:
        return x * 2;
    case 3:
        return x ^ 255;
    case 4:
        return -x;
    case 5:
        return x / 3;
    case 6:
        return x % 5;
    case 7:
        return x << 2;
    default:
        return x;
    }
}

int main(void) {
    /* Read through a volatile so the compiler cannot bake one x into
     * a table of results. The switch itself stays a jump table. */
    volatile int input = 30;
    int x = input;
    int expect[9] = {31, 29, 60, 30 ^ 255, -30, 10, 0, 120, 30};
    for (int key = -1; key <= 7; key++) {
        int got = dispatch(key, x);
        int want = (key < 0) ? expect[8] : expect[key];
        printf("dispatch(%d, %d) = %d\n", key, x, got);
        if (got != want) {
            printf("  expected %d\n", want);
            return 1;
        }
    }
    printf("switch checks passed\n");
    return 0;
}
