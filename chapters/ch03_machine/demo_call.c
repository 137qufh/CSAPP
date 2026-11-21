/* Calls, recursion, and the System V AMD64 stack frame.
 *
 * The first six integer arguments travel in
 *   %rdi, %rsi, %rdx, %rcx, %r8, %r9
 * and the return value comes back in %rax. Argument seven is stored
 * on the stack, just above the return address the call instruction
 * pushes.
 *
 * With a frame pointer (-fno-omit-frame-pointer, used by `make asm`):
 *
 *     pushq %rbp
 *     movq  %rsp, %rbp
 *     ...
 *     call  fact          # pushes the return address
 *     ...
 *     popq  %rbp
 *     ret
 *
 * Each recursive fact frame holds the saved %rbp, the return address,
 * and the local n that the multiply still needs after the call.
 */
#include <stdio.h>

__attribute__((noinline))
static long fact(int n) {
    if (n <= 1) {
        return 1;
    }
    return (long)n * fact(n - 1);
}

__attribute__((noinline))
static long fib(int n) {
    if (n <= 1) {
        return n;
    }
    return fib(n - 1) + fib(n - 2);
}

__attribute__((noinline))
static int sum7(int a, int b, int c, int d, int e, int f, int g) {
    return a + b + c + d + e + f + g;
}

int main(void) {
    long f6 = fact(6);
    long b10 = fib(10);
    int mixed = sum7(1, 2, 3, 4, 5, 6, 7);
    printf("fact(6)  = %ld\n", f6);
    printf("fact(1)  = %ld\n", fact(1));
    printf("fib(10)  = %ld\n", b10);
    printf("sum7     = %d\n", mixed);
    if (f6 != 720 || fact(0) != 1 || fact(1) != 1) {
        return 1;
    }
    if (b10 != 55 || fib(0) != 0 || fib(1) != 1) {
        return 1;
    }
    if (mixed != 28) {
        return 1;
    }
    printf("call checks passed\n");
    return 0;
}
