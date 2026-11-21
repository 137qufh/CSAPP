/* A software sketch of a stack canary, plus a look at ASLR.
 *
 * Compilers place a secret guard between local variables and the
 * saved return address. Before the function returns, it compares the
 * guard with the original secret. A mismatch means something overwrote
 * the guard, so the function must not continue as if the frame were
 * intact.
 *
 * This file does that comparison in ordinary C. It does not overflow a
 * buffer and it does not contain a payload. The secret here is a fixed
 * teaching value; a real canary is chosen at random when the process
 * starts.
 *
 * ASLR gives the stack a different base address on each execution.
 * Run the program twice and compare the addresses below. Nothing here
 * tries to predict or pin them down.
 */
#include <stdint.h>
#include <stdio.h>

static volatile uint32_t secret_guard = 0xA5A5F00Du;

__attribute__((noinline))
static int guarded_add(int x, int y) {
    uint32_t canary = secret_guard;
    int sum = x + y;
    uint32_t again = secret_guard;
    if (canary != again) {
        printf("guard mismatch (0x%08x vs 0x%08x); refusing the return\n", canary, again);
        return 0;
    }
    printf("guard intact (0x%08x); sum = %d\n", canary, sum);
    return sum;
}

__attribute__((noinline))
static int return_if_guard_matches(uint32_t found, int value) {
    uint32_t expect = secret_guard;
    if (found != expect) {
        printf("check: found 0x%08x, secret differs, value discarded\n", found);
        return 0;
    }
    printf("check: guard matches, returning %d\n", value);
    return value;
}

int main(void) {
    int local_a = 7;
    int local_b = 11;
    printf("stack local_a = %p\n", (void *)&local_a);
    printf("stack local_b = %p\n", (void *)&local_b);
    printf("code  main    = %p\n", (void *)(uintptr_t)main);
    printf("ASLR picks the stack location when the process starts.\n");
    printf("A canary is a secret checked before the function returns.\n");

    int sum = guarded_add(20, 22);
    int ok = return_if_guard_matches(secret_guard, sum);
    int bad = return_if_guard_matches(0u, sum);
    printf("guarded_add -> %d, match -> %d, mismatch -> %d\n", sum, ok, bad);
    if (sum != 42 || ok != 42 || bad != 0) {
        return 1;
    }
    return 0;
}
