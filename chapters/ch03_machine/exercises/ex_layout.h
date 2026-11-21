#ifndef EX_LAYOUT_H
#define EX_LAYOUT_H

/*
 * Natural alignment on x86-64 (LP64), not packed.
 *
 *     struct Slot {
 *         char  a;    // 1 byte, align 1
 *         int   b;    // 4 bytes, align 4
 *         short c;    // 2 bytes, align 2
 *     };
 *
 * Padding exists so every member starts at a multiple of its own
 * alignment, and so sizeof(struct Slot) is a multiple of the struct's
 * alignment. The struct alignment is the strictest member alignment,
 * which here is 4 because of the int. Without the gap in front of b,
 * an int load would be misaligned. Without the tail padding, an array
 * of Slot would place the next element's int on a non-multiple of 4.
 *
 * Do not define the struct in the .c file. Return the offsets and the
 * total span as plain numbers:
 *
 *     off_a()  byte offset of a
 *     off_b()  byte offset of b
 *     off_c()  byte offset of c
 *     span()   sizeof(struct Slot)
 *
 * align_up(value, align) is the smallest multiple of align that is
 * greater than or equal to value. align is a power of two (1, 2, 4,
 * 8, ...). value may be any int; the tests use non-negative values.
 * The same mask trick is how a compiler rounds a struct size up.
 */

int align_up(int value, int align);
int off_a(void);
int off_b(void);
int off_c(void);
int span(void);

#endif
