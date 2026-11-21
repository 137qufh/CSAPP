#include "bits.h"
#include "minitest.h"

#include <limits.h>
#include <math.h>
#include <string.h>

static unsigned bits_of(float f) {
    unsigned u;
    memcpy(&u, &f, sizeof(u));
    return u;
}

static unsigned ref_pop(unsigned x) {
    unsigned n = 0;
    while (x) {
        n += x & 1u;
        x >>= 1;
    }
    return n;
}

int main(void) {
    MT_EQ_INT(bitwise_or(0x0F0, 0x00F), 0x0FF);
    MT_EQ_INT(bitwise_or(0, 0), 0);
    MT_EQ_INT(bitwise_xor(0xFF00, 0x0FF0), 0xF0F0);
    MT_EQ_INT(bitwise_xor(7, 7), 0);
    MT_EQ_INT(bitwise_nand(0xF, 0x3), (int)~(0xF & 0x3));

    MT_EQ_UINT(low_mask(0), 0u);
    MT_EQ_UINT(low_mask(1), 1u);
    MT_EQ_UINT(low_mask(4), 0xFu);
    MT_EQ_UINT(low_mask(31), 0x7FFFFFFFu);

    MT_EQ_INT(is_zero(0), 1);
    MT_EQ_INT(is_zero(1), 0);
    MT_EQ_INT(is_zero(-1), 0);
    MT_EQ_INT(is_zero(INT_MIN), 0);

    MT_EQ_INT(is_negative(0), 0);
    MT_EQ_INT(is_negative(4), 0);
    MT_EQ_INT(is_negative(-1), 1);
    MT_EQ_INT(is_negative(INT_MIN), 1);

    MT_EQ_INT(sign_mask(3), 0);
    MT_EQ_INT(sign_mask(0), 0);
    MT_EQ_INT(sign_mask(-2), ~0);
    MT_EQ_INT(sign_mask(INT_MIN), ~0);

    MT_EQ_INT(abs_val(0), 0);
    MT_EQ_INT(abs_val(5), 5);
    MT_EQ_INT(abs_val(-5), 5);
    MT_EQ_INT(abs_val(INT_MIN), INT_MIN);

    MT_EQ_INT(isolate_lowbit(0), 0);
    MT_EQ_INT(isolate_lowbit(0xA0), 0x20);
    MT_EQ_INT(isolate_lowbit(1), 1);
    MT_EQ_INT(clear_lowbit(0), 0);
    MT_EQ_INT(clear_lowbit(0xA0), 0x80);
    MT_EQ_INT(clear_lowbit(1), 0);

    MT_EQ_INT(is_power2(0), 0);
    MT_EQ_INT(is_power2(1), 1);
    MT_EQ_INT(is_power2(8), 1);
    MT_EQ_INT(is_power2(12), 0);
    MT_EQ_INT(is_power2(-8), 0);
    MT_EQ_INT(is_power2(INT_MIN), 0);
    MT_EQ_INT(is_power2(1 << 20), 1);

    unsigned samples[] = {0u, 1u, 0x80000000u, 0xFFFFFFFFu, 0x12345678u, 7u};
    for (unsigned i = 0; i < sizeof(samples) / sizeof(samples[0]); i++) {
        MT_EQ_INT(odd_parity(samples[i]), (int)(ref_pop(samples[i]) & 1u));
        MT_EQ_UINT(popcount(samples[i]), ref_pop(samples[i]));
    }

    MT_EQ_UINT(rotate_left(0x12345678u, 0), 0x12345678u);
    MT_EQ_UINT(rotate_left(0x12345678u, 4), 0x23456781u);
    MT_EQ_UINT(rotate_left(0x80000001u, 1), 0x00000003u);
    MT_EQ_UINT(rotate_left(0xABCDEF01u, 31), (0xABCDEF01u << 31) | (0xABCDEF01u >> 1));

    MT_EQ_UINT(byte_reverse(0xAABBCCDDu), 0xDDCCBBAAu);
    MT_EQ_UINT(byte_reverse(0x01020304u), 0x04030201u);
    MT_EQ_UINT(swap_adjacent_bytes(0xAABBCCDDu), 0xBBAADDCCu);

    MT_EQ_INT(mul_by_15(0), 0);
    MT_EQ_INT(mul_by_15(3), 45);
    MT_EQ_INT(mul_by_15(-2), -30);

    MT_EQ_INT(logical_not(0), 1);
    MT_EQ_INT(logical_not(2), 0);
    MT_EQ_INT(logical_not(-1), 0);
    MT_EQ_INT(is_equal(3, 3), 1);
    MT_EQ_INT(is_equal(3, 4), 0);
    MT_EQ_INT(is_equal(INT_MIN, INT_MIN), 1);

    MT_EQ_INT(tmax_value(), INT_MAX);
    MT_EQ_INT(tmin_value(), INT_MIN);

    MT_EQ_INT(fits_short(0), 1);
    MT_EQ_INT(fits_short(32767), 1);
    MT_EQ_INT(fits_short(-32768), 1);
    MT_EQ_INT(fits_short(32768), 0);
    MT_EQ_INT(fits_short(-32769), 0);
    MT_EQ_INT(fits_short(INT_MIN), 0);

    unsigned one = bits_of(1.f);
    unsigned nan = 0x7FC00000u;
    unsigned nans = 0xFFC00011u;
    MT_EQ_INT(float_is_zero(0), 1);
    MT_EQ_INT(float_is_zero(0x80000000u), 1);
    MT_EQ_INT(float_is_zero(one), 0);
    MT_EQ_INT(float_is_denorm(1), 1);
    MT_EQ_INT(float_is_denorm(0), 0);
    MT_EQ_INT(float_is_denorm(one), 0);
    MT_EQ_INT(float_is_inf(0x7F800000u), 1);
    MT_EQ_INT(float_is_inf(0xFF800000u), 1);
    MT_EQ_INT(float_is_inf(nan), 0);
    MT_EQ_INT(float_is_nan(nan), 1);
    MT_EQ_INT(float_is_nan(nans), 1);
    MT_EQ_INT(float_is_nan(0x7F800000u), 0);

    MT_EQ_UINT(float_abs_bits(bits_of(-3.5f)), bits_of(3.5f));
    MT_EQ_UINT(float_neg_bits(one), bits_of(-1.f));
    MT_EQ_UINT(float_neg_bits(nans), nans);
    MT_EQ_UINT(float_neg_bits(0), 0x80000000u);

    float vf[] = {0.f, -0.f, 1.f, -1.f, 0.5f, 3.f, 1e-40f, 1e38f, -1e38f, 1.5f};
    for (unsigned i = 0; i < sizeof(vf) / sizeof(vf[0]); i++) {
        unsigned u = bits_of(vf[i]);
        MT_EQ_UINT(float_twice_bits(u), bits_of(ldexpf(vf[i], 1)));
        MT_EQ_UINT(float_half_bits(u), bits_of(ldexpf(vf[i], -1)));
    }
    MT_EQ_UINT(float_twice_bits(0x7F800000u), 0x7F800000u);
    MT_EQ_UINT(float_twice_bits(nan), nan);
    MT_EQ_UINT(float_half_bits(0xFF800000u), 0xFF800000u);
    MT_EQ_UINT(float_half_bits(nans), nans);

    MT_REPORT();
}
