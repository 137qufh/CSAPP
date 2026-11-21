#ifndef EX_FLOATBITS_H
#define EX_FLOATBITS_H

/* Bit-level binary32 helpers. Arguments are raw IEEE-754 bits.
 * Do not use floating types or libm inside the solutions; the tests compare
 * against a software reference.
 */

int float_is_zero(unsigned uf);
int float_is_denorm(unsigned uf);
int float_is_inf(unsigned uf);
int float_is_nan(unsigned uf);

/* Clear the sign bit. A NaN stays a NaN. */
unsigned float_abs_bits(unsigned uf);

/* Toggle the sign bit. A NaN is returned unchanged, sign included. */
unsigned float_neg_bits(unsigned uf);

/* Multiply by 2. Overflow becomes signed infinity. NaN/Inf stay as they are. */
unsigned float_twice_bits(unsigned uf);

/* Convert toward zero to int. Values that do not fit, and NaN/Inf, return
 * 0x80000000 as a distinguished failure code.
 */
int float_to_int(unsigned uf);

#endif
