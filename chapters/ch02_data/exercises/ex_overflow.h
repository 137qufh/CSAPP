#ifndef EX_OVERFLOW_H
#define EX_OVERFLOW_H

/* Return 1 when the mathematical result fits in a 32-bit int. */
int add_ok(int x, int y);
int sub_ok(int x, int y);
int mul_ok(int x, int y);

/* Two's-complement saturating add. */
int saturating_add(int x, int y);

#endif
