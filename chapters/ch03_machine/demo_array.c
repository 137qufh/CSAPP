/* Arrays, structs, unions, and alignment.
 *
 * An array of T is contiguous: &a[i] = base + i * sizeof(T).
 * A struct inserts padding so each field is naturally aligned and so
 * the size is a multiple of the struct alignment. A union overlays
 * its members; writing one and reading another reinterprets the bytes.
 */
#include <stddef.h>
#include <stdio.h>

struct Slot {
    char a;
    int b;
    short c;
};

struct Mixed {
    char tag;
    double value;
    short flag;
    char tail;
};

union Overlay {
    unsigned int word;
    unsigned char bytes[4];
    unsigned short halves[2];
};

int main(void) {
    int nums[4] = {10, 20, 30, 40};
    printf("sizeof(int)      = %zu  align = %zu\n", sizeof(int), _Alignof(int));
    printf("sizeof(nums)     = %zu\n", sizeof(nums));
    for (int i = 0; i < 4; i++) {
        printf("  nums[%d] at %p = %d\n", i, (void *)&nums[i], nums[i]);
    }
    printf("index step       = %ld bytes\n", (long)((char *)&nums[1] - (char *)&nums[0]));

    printf("sizeof(Slot)     = %zu  align = %zu\n", sizeof(struct Slot), _Alignof(struct Slot));
    printf("  a at %zu\n", offsetof(struct Slot, a));
    printf("  b at %zu\n", offsetof(struct Slot, b));
    printf("  c at %zu\n", offsetof(struct Slot, c));

    printf("sizeof(Mixed)    = %zu  align = %zu\n", sizeof(struct Mixed), _Alignof(struct Mixed));
    printf("  tag   at %zu\n", offsetof(struct Mixed, tag));
    printf("  value at %zu\n", offsetof(struct Mixed, value));
    printf("  flag  at %zu\n", offsetof(struct Mixed, flag));
    printf("  tail  at %zu\n", offsetof(struct Mixed, tail));

    struct Slot row[2];
    printf("Slot array stride = %ld\n", (long)((char *)&row[1] - (char *)&row[0]));

    union Overlay u;
    u.word = 0x11223344u;
    printf("sizeof(Overlay)  = %zu\n", sizeof(u));
    printf("word 0x%x bytes", u.word);
    for (int i = 0; i < 4; i++) {
        printf(" %02x", u.bytes[i]);
    }
    printf("  halves %04x %04x\n", u.halves[0], u.halves[1]);

    if (sizeof(struct Slot) != 12 || offsetof(struct Slot, a) != 0 || offsetof(struct Slot, b) != 4 ||
        offsetof(struct Slot, c) != 8) {
        printf("Slot layout is not the expected x86-64 layout\n");
        return 1;
    }
    if (u.bytes[0] != 0x44) {
        printf("expected little-endian byte 0\n");
        return 1;
    }
    printf("layout checks passed\n");
    return 0;
}
