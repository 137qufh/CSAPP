/* Hex, decimal, and binary views of the same value. */
#include <stdio.h>
#include <stdlib.h>

static void print_bits(unsigned x) {
    for (int i = 31; i >= 0; i--) {
        putchar((x & (1u << i)) ? '1' : '0');
        if (i % 4 == 0 && i != 0) {
            putchar(' ');
        }
    }
}

int main(int argc, char **argv) {
    unsigned long v = (argc > 1) ? strtoul(argv[1], NULL, 0) : 0xA5u;
    unsigned x = (unsigned)v;
    printf("dec  %u\n", x);
    printf("hex  0x%08X\n", x);
    printf("bin  ");
    print_bits(x);
    printf("\n");
    return 0;
}
