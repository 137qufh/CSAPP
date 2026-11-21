/* Decode and encode IEEE-754 binary32 values from their raw bits. */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

static void decode(uint32_t u) {
    unsigned sign = u >> 31;
    unsigned exp = (u >> 23) & 0xFF;
    unsigned frac = u & 0x7FFFFF;
    const char *kind = "normal";
    if (exp == 0 && frac == 0) {
        kind = "zero";
    } else if (exp == 0) {
        kind = "denormal";
    } else if (exp == 255 && frac == 0) {
        kind = "infinity";
    } else if (exp == 255) {
        kind = "nan";
    }
    float f;
    memcpy(&f, &u, sizeof(f));
    printf("0x%08X  sign=%u exp=%3u frac=0x%06X  %-8s  printf=%.9g\n", u, sign,
           exp, frac, kind, f);
}

static uint32_t encode_normal(int sign, int unbiased_exp, unsigned frac23) {
    unsigned exp = (unsigned)(unbiased_exp + 127);
    return ((unsigned)sign << 31) | (exp << 23) | (frac23 & 0x7FFFFF);
}

int main(void) {
    float samples[] = {0.f, -0.f, 1.f, -2.5f, 0.1f};
    for (unsigned i = 0; i < sizeof(samples) / sizeof(samples[0]); i++) {
        uint32_t u;
        memcpy(&u, &samples[i], sizeof(u));
        decode(u);
    }
    decode(0x7F800000u);
    decode(0xFF800000u);
    decode(0x7FC00000u);
    decode(0x00000001u); /* smallest positive denormal */
    decode(0x00800000u); /* smallest positive normal */

    uint32_t built = encode_normal(0, 0, 0); /* 1.0 */
    printf("encoded 1.0 -> 0x%08X\n", built);
    return 0;
}
