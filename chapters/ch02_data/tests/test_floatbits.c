#include "ex_floatbits.h"
#include "minitest.h"

#include <string.h>

static unsigned bits_of(float f) {
    unsigned u;
    memcpy(&u, &f, sizeof(u));
    return u;
}

int main(void) {
    unsigned one = bits_of(1.f);
    unsigned neg = bits_of(-2.f);
    unsigned nan = 0x7FC00000u;
    unsigned nans = 0xFFC00001u;
    unsigned pinf = 0x7F800000u;
    unsigned ninf = 0xFF800000u;
    unsigned den = 0x00000001u;
    unsigned zero = 0;
    unsigned nzero = 0x80000000u;

    MT_EQ_INT(float_is_zero(zero), 1);
    MT_EQ_INT(float_is_zero(nzero), 1);
    MT_EQ_INT(float_is_zero(one), 0);
    MT_EQ_INT(float_is_denorm(den), 1);
    MT_EQ_INT(float_is_denorm(zero), 0);
    MT_EQ_INT(float_is_inf(pinf), 1);
    MT_EQ_INT(float_is_inf(ninf), 1);
    MT_EQ_INT(float_is_inf(nan), 0);
    MT_EQ_INT(float_is_nan(nan), 1);
    MT_EQ_INT(float_is_nan(nans), 1);
    MT_EQ_INT(float_is_nan(pinf), 0);

    MT_EQ_UINT(float_abs_bits(neg), bits_of(2.f));
    MT_EQ_UINT(float_abs_bits(nan) & 0x7FFFFFFFu, nan & 0x7FFFFFFFu);
    MT_EQ_UINT(float_neg_bits(one), bits_of(-1.f));
    MT_EQ_UINT(float_neg_bits(nans), nans);
    MT_EQ_UINT(float_neg_bits(zero), nzero);

    MT_EQ_UINT(float_twice_bits(one), bits_of(2.f));
    MT_EQ_UINT(float_twice_bits(bits_of(0.5f)), one);
    MT_EQ_UINT(float_twice_bits(pinf), pinf);
    MT_EQ_UINT(float_twice_bits(nan), nan);
    MT_EQ_UINT(float_twice_bits(zero), zero);
    MT_EQ_UINT(float_twice_bits(nzero), nzero);
    MT_EQ_UINT(float_twice_bits(bits_of(2e38f)) & 0x7F800000u, 0x7F800000u);

    /* denormal * 2 */
    MT_EQ_UINT(float_twice_bits(den), den << 1);

    MT_EQ_INT(float_to_int(one), 1);
    MT_EQ_INT(float_to_int(bits_of(-2.9f)), -2);
    MT_EQ_INT(float_to_int(bits_of(0.9f)), 0);
    MT_EQ_INT(float_to_int(bits_of(-0.9f)), 0);
    MT_EQ_INT(float_to_int(pinf), (int)0x80000000u);
    MT_EQ_INT(float_to_int(nan), (int)0x80000000u);
    MT_EQ_INT(float_to_int(bits_of(1024.f)), 1024);
    MT_EQ_INT(float_to_int(bits_of(-2147483648.f)), (int)0x80000000u);
    MT_REPORT();
}
