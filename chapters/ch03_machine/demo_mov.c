/* Data movement and lea-style address arithmetic.
 *
 * A mov copies bits between registers or between a register and memory.
 * movl writes the low 32 bits and clears the high half of a 64-bit
 * register; a full movq copies all 64.
 *
 * The address of cells[i] is base + i * sizeof(int). The compiler
 * usually emits one lea and does not read the cell:
 *
 *     leaq (%rdi,%rsi,4), %rax     # rax = rdi + rsi*4
 */
#include <stdint.h>
#include <stdio.h>

__attribute__((noinline))
static uintptr_t scaled_address(uintptr_t base, int index, int scale) {
    return base + (uintptr_t)index * (uintptr_t)scale;
}

__attribute__((noinline))
static int low32(long wide) {
    return (int)wide;
}

int main(void) {
    int cells[6] = {10, 20, 30, 40, 50, 60};
    int index = 3;
    uintptr_t base = (uintptr_t)cells;
    uintptr_t manual = scaled_address(base, index, (int)sizeof(int));
    uintptr_t via_c = (uintptr_t)&cells[index];

    printf("base              = %p\n", (void *)cells);
    printf("index %d scale %zu\n", index, sizeof(int));
    printf("lea-style address = %p\n", (void *)manual);
    printf("&cells[%d]        = %p\n", index, (void *)via_c);
    printf("value             = %d\n", *(int *)manual);

    long wide = 0x1122334455667788L;
    long copied = wide; /* movq-style: the whole register */
    int chopped = low32(wide);
    printf("movq copy         = 0x%lx\n", (unsigned long)copied);
    printf("movl low 32 bits  = 0x%x\n", (unsigned)chopped);

    if (manual != via_c || *(int *)manual != 40) {
        printf("address check failed\n");
        return 1;
    }
    if (copied != wide || (unsigned)chopped != 0x55667788u) {
        printf("move check failed\n");
        return 1;
    }
    printf("mov checks passed\n");
    return 0;
}
