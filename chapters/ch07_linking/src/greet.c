#include "greet.h"

#include <stdio.h>

void greet(const char *name) {
    if (!name) {
        name = "?";
    }
    printf("hello, %s\n", name);
}
