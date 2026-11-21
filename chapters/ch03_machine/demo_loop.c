/* Loops lower to a compare and a conditional jump.
 *
 * A for/while test at the top is a jmp to the condition, then the
 * body, then a jcc back. A do-while tests at the bottom, so the body
 * always runs once. Summing 1..n is the same arithmetic either way.
 */
#include <stdio.h>

__attribute__((noinline))
static int sum_for(int n) {
    int sum = 0;
    for (int i = 1; i <= n; i++) {
        sum += i;
    }
    return sum;
}

__attribute__((noinline))
static int sum_while(int n) {
    int sum = 0;
    int i = 1;
    while (i <= n) {
        sum += i;
        i++;
    }
    return sum;
}

__attribute__((noinline))
static int sum_do(int n) {
    int sum = 0;
    int i = 1;
    if (n <= 0) {
        return 0;
    }
    do {
        sum += i;
        i++;
    } while (i <= n);
    return sum;
}

int main(void) {
    int n = 10;
    int a = sum_for(n);
    int b = sum_while(n);
    int c = sum_do(n);
    printf("sum_for(%d)   = %d\n", n, a);
    printf("sum_while(%d) = %d\n", n, b);
    printf("sum_do(%d)    = %d\n", n, c);
    printf("sum_for(0)    = %d\n", sum_for(0));
    printf("sum_do(0)     = %d\n", sum_do(0));
    printf("sum_for(1)    = %d\n", sum_for(1));
    if (a != 55 || b != 55 || c != 55) {
        return 1;
    }
    if (sum_for(0) != 0 || sum_do(0) != 0 || sum_while(0) != 0 || sum_for(1) != 1) {
        return 1;
    }
    printf("loop checks passed\n");
    return 0;
}
