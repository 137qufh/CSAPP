/* Print the object representation of a few values, one byte at a time.
 * On little-endian x86-64 the least significant byte appears first.
 */
#include <stdio.h>

static void show_bytes(const char *label, const void *p, size_t n) {
    const unsigned char *b = p;
    printf("%-12s", label);
    for (size_t i = 0; i < n; i++) {
        printf(" %02X", b[i]);
    }
    printf("\n");
}

int main(void) {
    int i = 0x12345678;
    float f = 3.140625f; /* exact in binary: 3 + 1/8 + 1/64 */
    short s = -2;
    const char *str = "Hi";

    show_bytes("int", &i, sizeof(i));
    show_bytes("float", &f, sizeof(f));
    show_bytes("short", &s, sizeof(s));
    show_bytes("chars", str, 3); /* H i \0 */

    unsigned w = 1u;
    show_bytes("endian 1", &w, sizeof(w));
    if (((unsigned char *)&w)[0] == 1) {
        printf("byte order: little-endian\n");
    } else {
        printf("byte order: big-endian\n");
    }
    return 0;
}
