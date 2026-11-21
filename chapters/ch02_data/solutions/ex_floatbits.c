#include "ex_floatbits.h"

static unsigned fexp(unsigned uf) { return (uf >> 23) & 0xFFu; }
static unsigned ffrac(unsigned uf) { return uf & 0x7FFFFFu; }
static unsigned fsign(unsigned uf) { return uf & 0x80000000u; }

int float_is_zero(unsigned uf) { return fexp(uf) == 0 && ffrac(uf) == 0; }
int float_is_denorm(unsigned uf) { return fexp(uf) == 0 && ffrac(uf) != 0; }
int float_is_inf(unsigned uf) { return fexp(uf) == 0xFFu && ffrac(uf) == 0; }
int float_is_nan(unsigned uf) { return fexp(uf) == 0xFFu && ffrac(uf) != 0; }

unsigned float_abs_bits(unsigned uf) { return uf & 0x7FFFFFFFu; }

unsigned float_neg_bits(unsigned uf) {
    if (float_is_nan(uf)) {
        return uf;
    }
    return uf ^ 0x80000000u;
}

unsigned float_twice_bits(unsigned uf) {
    unsigned sign = fsign(uf);
    unsigned exp = fexp(uf);
    unsigned frac = ffrac(uf);
    if (exp == 0xFFu) {
        return uf;
    }
    if (exp == 0) {
        frac <<= 1;
        if (frac & 0x800000u) {
            exp = 1;
            frac &= 0x7FFFFFu;
        }
        return sign | (exp << 23) | frac;
    }
    exp += 1;
    if (exp == 0xFFu) {
        return sign | 0x7F800000u;
    }
    return sign | (exp << 23) | frac;
}

int float_to_int(unsigned uf) {
    unsigned sign = uf >> 31;
    unsigned expb = fexp(uf);
    unsigned frac = ffrac(uf);
    if (expb == 0xFFu) {
        return (int)0x80000000u;
    }
    if (expb == 0) {
        return 0; /* denormals are < 1 in magnitude for binary32 */
    }
    int exp = (int)expb - 127;
    if (exp < 0) {
        return 0;
    }
    if (exp > 31) {
        return (int)0x80000000u;
    }
    unsigned mag = (1u << 23) | frac;
    unsigned shifted;
    if (exp >= 23) {
        if (exp - 23 >= 31) {
            return (int)0x80000000u;
        }
        shifted = mag << (exp - 23);
    } else {
        shifted = mag >> (23 - exp);
    }
    if (!sign) {
        if (shifted > 0x7FFFFFFFu) {
            return (int)0x80000000u;
        }
        return (int)shifted;
    }
    if (shifted > 0x80000000u) {
        return (int)0x80000000u;
    }
    if (shifted == 0x80000000u) {
        return (int)0x80000000u; /* INT_MIN */
    }
    return (int)(~shifted + 1u);
}
