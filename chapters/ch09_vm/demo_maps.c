#include <stdio.h>

/* Print a short reading of /proc/self/maps: code, data, heap, stack. */
int main(void) {
    FILE *f = fopen("/proc/self/maps", "r");
    if (!f) {
        perror("/proc/self/maps");
        return 1;
    }
    char line[256];
    int n = 0;
    while (fgets(line, sizeof(line), f) && n < 12) {
        fputs(line, stdout);
        n++;
    }
    fclose(f);
    printf("maps-lines=%d\n", n);
    return n > 0 ? 0 : 1;
}
