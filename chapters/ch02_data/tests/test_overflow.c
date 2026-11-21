#include "ex_overflow.h"
#include "minitest.h"

#include <limits.h>

int main(void) {
    MT_EQ_INT(add_ok(1, 2), 1);
    MT_EQ_INT(add_ok(INT_MAX, 0), 1);
    MT_EQ_INT(add_ok(INT_MAX, 1), 0);
    MT_EQ_INT(add_ok(INT_MIN, -1), 0);
    MT_EQ_INT(add_ok(INT_MIN, 0), 1);
    MT_EQ_INT(add_ok(-4, -5), 1);
    MT_EQ_INT(add_ok(INT_MIN, INT_MIN), 0);

    MT_EQ_INT(sub_ok(5, 3), 1);
    MT_EQ_INT(sub_ok(INT_MIN, 1), 0);
    MT_EQ_INT(sub_ok(INT_MAX, -1), 0);
    MT_EQ_INT(sub_ok(INT_MIN, INT_MIN), 1);
    MT_EQ_INT(sub_ok(0, INT_MIN), 0);
    MT_EQ_INT(sub_ok(-1, INT_MIN), 1);

    MT_EQ_INT(mul_ok(3, 4), 1);
    MT_EQ_INT(mul_ok(INT_MAX, 2), 0);
    MT_EQ_INT(mul_ok(INT_MIN, -1), 0);
    MT_EQ_INT(mul_ok(INT_MIN, 1), 1);
    MT_EQ_INT(mul_ok(-100, 0), 1);
    MT_EQ_INT(mul_ok(46341, 46341), 0);

    MT_EQ_INT(saturating_add(10, 20), 30);
    MT_EQ_INT(saturating_add(INT_MAX, 5), INT_MAX);
    MT_EQ_INT(saturating_add(INT_MIN, -5), INT_MIN);
    MT_EQ_INT(saturating_add(INT_MIN, INT_MAX), -1);
    MT_REPORT();
}
