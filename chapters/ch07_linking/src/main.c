#include "ops.h"

#include <stdio.h>

/* Linked from three separately compiled objects. */
int main(void) {
    printf("sum=%d product=%d\n", add(3, 4), mul(3, 4));
    return 0;
}
