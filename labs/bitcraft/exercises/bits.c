#include "bits.h"

/* Student starter. Replace each TODO. Keep the puzzle comment above the
 * function: opcheck.py enforces it. See README.md for the rules.
 */

/* puzzle: bitwise_or
 * allow: ~ &
 * max_ops: 8
 * const_max: 0
 * control: no
 */
int bitwise_or(int x, int y) {
    (void)x;
    (void)y;
    return 0; /* TODO */
}

/* puzzle: bitwise_xor
 * allow: ~ &
 * max_ops: 14
 * const_max: 0
 * control: no
 */
int bitwise_xor(int x, int y) {
    (void)x;
    (void)y;
    return 0; /* TODO */
}

/* puzzle: bitwise_nand
 * allow: ~ &
 * max_ops: 4
 * const_max: 0
 * control: no
 */
int bitwise_nand(int x, int y) {
    (void)x;
    (void)y;
    return 0; /* TODO */
}

/* puzzle: low_mask
 * allow: ~ <<
 * max_ops: 6
 * const_max: 0
 * control: no
 * note: 0 <= n <= 31, n ones in the low bits
 */
unsigned low_mask(int n) {
    (void)n;
    return 0; /* TODO */
}

/* puzzle: is_zero
 * allow: ~ | + >> ^
 * max_ops: 10
 * const_max: 31
 * control: no
 */
int is_zero(int x) {
    (void)x;
    return 0; /* TODO */
}

/* puzzle: is_negative
 * allow: >>
 * max_ops: 4
 * const_max: 31
 * control: no
 */
int is_negative(int x) {
    (void)x;
    return 0; /* TODO */
}

/* puzzle: sign_mask
 * allow: - >>
 * max_ops: 6
 * const_max: 31
 * control: no
 * note: 0 when x >= 0, all-bits-one when x < 0
 */
int sign_mask(int x) {
    (void)x;
    return 0; /* TODO */
}

/* puzzle: abs_val
 * allow: - ^ >> + ~
 * max_ops: 12
 * const_max: 31
 * control: no
 * note: INT_MIN stays INT_MIN (wraps)
 */
int abs_val(int x) {
    (void)x;
    return 0; /* TODO */
}

/* puzzle: isolate_lowbit
 * allow: & ~ +
 * max_ops: 6
 * const_max: 1
 * control: no
 */
int isolate_lowbit(int x) {
    (void)x;
    return 0; /* TODO */
}

/* puzzle: clear_lowbit
 * allow: & + ~
 * max_ops: 6
 * const_max: 0
 * control: no
 */
int clear_lowbit(int x) {
    (void)x;
    return 0; /* TODO */
}

/* puzzle: is_power2
 * allow: ! & + ~ | >>
 * max_ops: 16
 * const_max: 31
 * control: no
 * note: positive powers of two only (INT_MIN is not)
 */
int is_power2(int x) {
    (void)x;
    return 0; /* TODO */
}

/* puzzle: odd_parity
 * allow: ^ >> &
 * max_ops: 20
 * const_max: 255
 * control: no
 */
int odd_parity(unsigned x) {
    (void)x;
    return 0; /* TODO */
}

/* puzzle: rotate_left
 * allow: << >> | & -
 * max_ops: 12
 * const_max: 32
 * control: no
 * note: 0 <= n <= 31
 */
unsigned rotate_left(unsigned x, int n) {
    (void)x;
    (void)n;
    return 0; /* TODO */
}

/* puzzle: byte_reverse
 * allow: & >> << |
 * max_ops: 30
 * const_max: 255
 * control: no
 */
unsigned byte_reverse(unsigned x) {
    (void)x;
    return 0; /* TODO */
}

/* puzzle: swap_adjacent_bytes
 * allow: & >> << |
 * max_ops: 30
 * const_max: 255
 * control: no
 */
unsigned swap_adjacent_bytes(unsigned x) {
    (void)x;
    return 0; /* TODO */
}

/* puzzle: mul_by_15
 * allow: << + ~
 * max_ops: 8
 * const_max: 4
 * control: no
 */
int mul_by_15(int x) {
    (void)x;
    return 0; /* TODO */
}

/* puzzle: logical_not
 * allow: ~ | + >> ^
 * max_ops: 12
 * const_max: 31
 * control: no
 */
int logical_not(int x) {
    (void)x;
    return 0; /* TODO */
}

/* puzzle: is_equal
 * allow: ^ ~ | + >>
 * max_ops: 16
 * const_max: 31
 * control: no
 */
int is_equal(int x, int y) {
    (void)x;
    (void)y;
    return 0; /* TODO */
}

/* puzzle: tmax_value
 * allow: ~ <<
 * max_ops: 4
 * const_max: 31
 * control: no
 */
int tmax_value(void) {
    return 0; /* TODO */
}

/* puzzle: tmin_value
 * allow: <<
 * max_ops: 4
 * const_max: 31
 * control: no
 */
int tmin_value(void) {
    return 0; /* TODO */
}

/* puzzle: popcount
 * allow: & + >>
 * max_ops: 60
 * const_max: 4294967295
 * control: no
 */
unsigned popcount(unsigned x) {
    (void)x;
    return 0; /* TODO */
}

/* puzzle: fits_short
 * allow: << >> ^ !
 * max_ops: 12
 * const_max: 255
 * control: no
 * note: 1 iff x fits in a signed 16-bit short
 */
int fits_short(int x) {
    (void)x;
    return 0; /* TODO */
}

/* puzzle: float_is_zero
 * allow: & ==
 * max_ops: 6
 * const_max: 2147483647
 * control: no
 */
int float_is_zero(unsigned uf) {
    (void)uf;
    return 0; /* TODO */
}

/* puzzle: float_is_denorm
 * allow: & == !=
 * max_ops: 16
 * const_max: 2147483647
 * control: no
 */
int float_is_denorm(unsigned uf) {
    (void)uf;
    return 0; /* TODO */
}

/* puzzle: float_is_inf
 * allow: & ==
 * max_ops: 8
 * const_max: 2147483647
 * control: no
 */
int float_is_inf(unsigned uf) {
    (void)uf;
    return 0; /* TODO */
}

/* puzzle: float_is_nan
 * allow: & >
 * max_ops: 8
 * const_max: 2147483647
 * control: no
 */
int float_is_nan(unsigned uf) {
    (void)uf;
    return 0; /* TODO */
}

/* puzzle: float_abs_bits
 * allow: &
 * max_ops: 4
 * const_max: 2147483647
 * control: no
 */
unsigned float_abs_bits(unsigned uf) {
    (void)uf;
    return 0; /* TODO */
}

/* puzzle: float_neg_bits
 * allow: & > ^
 * max_ops: 20
 * const_max: 2147483648
 * control: yes
 */
unsigned float_neg_bits(unsigned uf) {
    (void)uf;
    return 0; /* TODO */
}

/* puzzle: float_twice_bits
 * allow: >> & | << + == !=
 * max_ops: 80
 * const_max: 2147483648
 * control: yes
 */
unsigned float_twice_bits(unsigned uf) {
    (void)uf;
    return 0; /* TODO */
}

/* puzzle: float_half_bits
 * allow: >> & | << + == != -
 * max_ops: 120
 * const_max: 2147483648
 * control: yes
 * note: round ties to even
 */
unsigned float_half_bits(unsigned uf) {
    (void)uf;
    return 0; /* TODO */
}
