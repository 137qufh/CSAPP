#include "ex_layout.h"

/* a occupies byte 0.
 * b needs a multiple of 4, so bytes 1..3 are padding and b is at 4.
 * c needs a multiple of 2; offset 8 is fine, so c is at 8.
 * The next free byte is 10. Struct alignment is 4, and align_up(10, 4)
 * is 12. Those two tail bytes keep an array of the struct aligned.
 */

int align_up(int value, int align) {
    int mask = align - 1;
    return (value + mask) & ~mask;
}

int off_a(void) { return 0; }

int off_b(void) { return 4; }

int off_c(void) { return 8; }

int span(void) { return 12; }
