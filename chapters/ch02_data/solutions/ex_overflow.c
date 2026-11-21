#include "ex_overflow.h"

#include <limits.h>

int add_ok(int x, int y) {
    int s = (int)((unsigned)x + (unsigned)y);
    if (x > 0 && y > 0 && s <= 0) {
        return 0;
    }
    if (x < 0 && y < 0 && s >= 0) {
        return 0;
    }
    return 1;
}

int sub_ok(int x, int y) {
    if (y == INT_MIN) {
        /* x - INT_MIN fits only when x is negative (including INT_MIN). */
        return x < 0;
    }
    return add_ok(x, -y);
}

int mul_ok(int x, int y) {
    long long p = (long long)x * (long long)y;
    return p == (int)p;
}

int saturating_add(int x, int y) {
    if (!add_ok(x, y)) {
        return (x < 0) ? INT_MIN : INT_MAX;
    }
    return x + y;
}
