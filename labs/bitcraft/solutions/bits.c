#include "bits.h"

/* puzzle: bitwise_or
 * allow: ~ &
 * max_ops: 8
 * const_max: 0
 * control: no
 */
int bitwise_or(int x, int y) { return ~(~x & ~y); }

/* puzzle: bitwise_xor
 * allow: ~ &
 * max_ops: 14
 * const_max: 0
 * control: no
 */
int bitwise_xor(int x, int y) { return ~(~(~x & y) & ~(x & ~y)); }

/* puzzle: bitwise_nand
 * allow: ~ &
 * max_ops: 4
 * const_max: 0
 * control: no
 */
int bitwise_nand(int x, int y) { return ~(x & y); }

/* puzzle: low_mask
 * allow: ~ <<
 * max_ops: 6
 * const_max: 0
 * control: no
 * note: 0 <= n <= 31, n ones in the low bits
 */
unsigned low_mask(int n) { return ~(~0u << n); }

/* puzzle: is_zero
 * allow: ~ | + >> ^
 * max_ops: 10
 * const_max: 31
 * control: no
 */
int is_zero(int x) {
    return (int)((((unsigned)x | (~(unsigned)x + 1u)) >> 31) ^ 1u);
}

/* puzzle: is_negative
 * allow: >>
 * max_ops: 4
 * const_max: 31
 * control: no
 */
int is_negative(int x) { return (int)((unsigned)x >> 31); }

/* puzzle: sign_mask
 * allow: - >>
 * max_ops: 6
 * const_max: 31
 * control: no
 * note: 0 when x >= 0, all-bits-one when x < 0
 */
int sign_mask(int x) { return (int)(0u - ((unsigned)x >> 31)); }

/* puzzle: abs_val
 * allow: - ^ >> + ~
 * max_ops: 12
 * const_max: 31
 * control: no
 * note: INT_MIN stays INT_MIN (wraps)
 */
int abs_val(int x) {
    int m = (int)(0u - ((unsigned)x >> 31));
    return (x ^ m) + (~m + 1);
}

/* puzzle: isolate_lowbit
 * allow: & ~ +
 * max_ops: 6
 * const_max: 1
 * control: no
 */
int isolate_lowbit(int x) { return x & (~x + 1); }

/* puzzle: clear_lowbit
 * allow: & + ~
 * max_ops: 6
 * const_max: 0
 * control: no
 */
int clear_lowbit(int x) { return x & (x + ~0); }

/* puzzle: is_power2
 * allow: ! & + ~ | >>
 * max_ops: 16
 * const_max: 31
 * control: no
 * note: positive powers of two only (INT_MIN is not)
 */
int is_power2(int x) {
    return !((x & (x + ~0)) | !x | (int)((unsigned)x >> 31));
}

/* puzzle: odd_parity
 * allow: ^ >> &
 * max_ops: 20
 * const_max: 255
 * control: no
 */
int odd_parity(unsigned x) {
    x = x ^ (x >> 16);
    x = x ^ (x >> 8);
    x = x ^ (x >> 4);
    x = x ^ (x >> 2);
    x = x ^ (x >> 1);
    return (int)(x & 1u);
}

/* puzzle: rotate_left
 * allow: << >> | & -
 * max_ops: 12
 * const_max: 32
 * control: no
 * note: 0 <= n <= 31
 */
unsigned rotate_left(unsigned x, int n) {
    return (x << n) | (x >> ((32 - n) & 31));
}

/* puzzle: byte_reverse
 * allow: & >> << |
 * max_ops: 30
 * const_max: 255
 * control: no
 */
unsigned byte_reverse(unsigned x) {
    unsigned b0 = x & 0xFFu;
    unsigned b1 = (x >> 8) & 0xFFu;
    unsigned b2 = (x >> 16) & 0xFFu;
    unsigned b3 = (x >> 24) & 0xFFu;
    return (b0 << 24) | (b1 << 16) | (b2 << 8) | b3;
}

/* puzzle: swap_adjacent_bytes
 * allow: & >> << |
 * max_ops: 30
 * const_max: 255
 * control: no
 */
unsigned swap_adjacent_bytes(unsigned x) {
    unsigned m = 0xFFu;
    return ((x & m) << 8) | ((x >> 8) & m) | (((x >> 16) & m) << 24) |
           (((x >> 24) & m) << 16);
}

/* puzzle: mul_by_15
 * allow: << + ~
 * max_ops: 8
 * const_max: 4
 * control: no
 */
int mul_by_15(int x) { return (int)(((unsigned)x << 4) + (~(unsigned)x + 1u)); }

/* puzzle: logical_not
 * allow: ~ | + >> ^
 * max_ops: 12
 * const_max: 31
 * control: no
 */
int logical_not(int x) {
    return (int)((((unsigned)x | (~(unsigned)x + 1u)) >> 31) ^ 1u);
}

/* puzzle: is_equal
 * allow: ^ ~ | + >>
 * max_ops: 16
 * const_max: 31
 * control: no
 */
int is_equal(int x, int y) {
    unsigned d = (unsigned)(x ^ y);
    return (int)(((d | (~d + 1u)) >> 31) ^ 1u);
}

/* puzzle: tmax_value
 * allow: ~ <<
 * max_ops: 4
 * const_max: 31
 * control: no
 */
int tmax_value(void) { return (int)~(1u << 31); }

/* puzzle: tmin_value
 * allow: <<
 * max_ops: 4
 * const_max: 31
 * control: no
 */
int tmin_value(void) { return (int)(1u << 31); }

/* puzzle: popcount
 * allow: & + >>
 * max_ops: 60
 * const_max: 4294967295
 * control: no
 */
unsigned popcount(unsigned x) {
    unsigned m1 = 0x55555555u;
    unsigned m2 = 0x33333333u;
    unsigned m4 = 0x0F0F0F0Fu;
    unsigned m8 = 0x00FF00FFu;
    unsigned m16 = 0x0000FFFFu;
    x = (x & m1) + ((x >> 1) & m1);
    x = (x & m2) + ((x >> 2) & m2);
    x = (x & m4) + ((x >> 4) & m4);
    x = (x & m8) + ((x >> 8) & m8);
    x = (x & m16) + ((x >> 16) & m16);
    return x;
}

/* puzzle: fits_short
 * allow: << >> ^ !
 * max_ops: 12
 * const_max: 255
 * control: no
 * note: 1 iff x fits in a signed 16-bit short
 */
int fits_short(int x) {
    int y = ((int)((unsigned)x << 16)) >> 16;
    return !(y ^ x);
}

/* puzzle: float_is_zero
 * allow: & ==
 * max_ops: 6
 * const_max: 2147483647
 * control: no
 */
int float_is_zero(unsigned uf) { return (uf & 0x7FFFFFFFu) == 0; }

/* puzzle: float_is_denorm
 * allow: & == !=
 * max_ops: 16
 * const_max: 2147483647
 * control: no
 */
int float_is_denorm(unsigned uf) {
    unsigned body = uf & 0x7FFFFFFFu;
    return (body != 0) & ((body & 0x7F800000u) == 0);
}

/* puzzle: float_is_inf
 * allow: & ==
 * max_ops: 8
 * const_max: 2147483647
 * control: no
 */
int float_is_inf(unsigned uf) { return (uf & 0x7FFFFFFFu) == 0x7F800000u; }

/* puzzle: float_is_nan
 * allow: & >
 * max_ops: 8
 * const_max: 2147483647
 * control: no
 */
int float_is_nan(unsigned uf) { return (uf & 0x7FFFFFFFu) > 0x7F800000u; }

/* puzzle: float_abs_bits
 * allow: &
 * max_ops: 4
 * const_max: 2147483647
 * control: no
 */
unsigned float_abs_bits(unsigned uf) { return uf & 0x7FFFFFFFu; }

/* puzzle: float_neg_bits
 * allow: & > ^
 * max_ops: 20
 * const_max: 2147483648
 * control: yes
 */
unsigned float_neg_bits(unsigned uf) {
    if ((uf & 0x7FFFFFFFu) > 0x7F800000u) {
        return uf;
    }
    return uf ^ 0x80000000u;
}

/* puzzle: float_twice_bits
 * allow: >> & | << + == !=
 * max_ops: 80
 * const_max: 2147483648
 * control: yes
 */
unsigned float_twice_bits(unsigned uf) {
    unsigned sign = uf & 0x80000000u;
    unsigned exp = (uf >> 23) & 0xFFu;
    unsigned frac = uf & 0x7FFFFFu;
    if (exp == 255) {
        return uf;
    }
    if (exp == 0) {
        frac = frac << 1;
        if (frac & 0x800000u) {
            exp = 1;
            frac = frac & 0x7FFFFFu;
        }
        return sign | (exp << 23) | frac;
    }
    exp = exp + 1;
    if (exp == 255) {
        return sign | 0x7F800000u;
    }
    return sign | (exp << 23) | frac;
}

/* puzzle: float_half_bits
 * allow: >> & | << + == != -
 * max_ops: 120
 * const_max: 2147483648
 * control: yes
 * note: round ties to even
 */
unsigned float_half_bits(unsigned uf) {
    unsigned sign = uf & 0x80000000u;
    unsigned exp = (uf >> 23) & 0xFFu;
    unsigned frac = uf & 0x7FFFFFu;
    unsigned mag;
    unsigned lost;
    if (exp == 255) {
        return uf;
    }
    if (exp == 0) {
        lost = frac & 1u;
        frac = frac >> 1;
        if (lost & (frac & 1u)) {
            frac = frac + 1u;
        }
        return sign | frac;
    }
    if (exp == 1) {
        mag = frac | 0x800000u;
        lost = mag & 1u;
        mag = mag >> 1;
        if (lost & (mag & 1u)) {
            mag = mag + 1u;
        }
        if (mag & 0x800000u) {
            return sign | (1u << 23);
        }
        return sign | mag;
    }
    return sign | ((exp - 1u) << 23) | frac;
}
