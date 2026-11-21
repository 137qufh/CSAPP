/* CGI-style greeting. Reads QUERY_STRING and prints a body only. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    const char *q = getenv("QUERY_STRING");
    char name[64];
    const char *value = "world";
    size_t i = 0;

    memcpy(name, "world", 6);
    if (q) {
        const char *p = strstr(q, "name=");
        if (p) {
            p += 5;
            while (p[i] != '\0' && p[i] != '&' && i + 1 < sizeof(name)) {
                name[i] = p[i];
                i++;
            }
            name[i] = '\0';
            if (name[0] != '\0') {
                value = name;
            }
        }
    }
    printf("hello, %s\n", value);
    if (fflush(stdout) != 0) {
        return 1;
    }
    return 0;
}
