#ifndef BITCRAFT_BITS_H
#define BITCRAFT_BITS_H

/* Bitcraft puzzles. Implementations go in bits.c.
 * Each puzzle comment states the only operators you may use.
 * The checker (opcheck.py) enforces that list. Functional tests check results.
 *
 * Unless a comment says control: yes, do not use if/else/loops/?:/&&/||.
 * Do not call other functions. Integer literals must fit in const_max.
 */

int bitwise_or(int x, int y);
int bitwise_xor(int x, int y);
int bitwise_nand(int x, int y);
unsigned low_mask(int n);
int is_zero(int x);
int is_negative(int x);
int sign_mask(int x);
int abs_val(int x);
int isolate_lowbit(int x);
int clear_lowbit(int x);
int is_power2(int x);
int odd_parity(unsigned x);
unsigned rotate_left(unsigned x, int n);
unsigned byte_reverse(unsigned x);
unsigned swap_adjacent_bytes(unsigned x);
int mul_by_15(int x);
int logical_not(int x);
int is_equal(int x, int y);
int tmax_value(void);
int tmin_value(void);
unsigned popcount(unsigned x);
int fits_short(int x);

int float_is_zero(unsigned uf);
int float_is_denorm(unsigned uf);
int float_is_inf(unsigned uf);
int float_is_nan(unsigned uf);
unsigned float_abs_bits(unsigned uf);
unsigned float_neg_bits(unsigned uf);
unsigned float_twice_bits(unsigned uf);
unsigned float_half_bits(unsigned uf);

#endif
