#include "ex_amdahl.h"
#include "minitest.h"

#include <math.h>

static int close_enough(double a, double b) {
    double d = a - b;
    if (d < 0) {
        d = -d;
    }
    return d < 1e-9;
}

int main(void) {
    MT_ASSERT(close_enough(amdahl_speedup(0.0, 100.0), 1.0));
    MT_ASSERT(close_enough(amdahl_speedup(1.0, 4.0), 4.0));
    MT_ASSERT(close_enough(amdahl_speedup(0.5, 2.0), 1.0 / 0.75));

    MT_ASSERT(close_enough(amdahl_required_factor(0.5, 1.6), 4.0));
    MT_ASSERT(amdahl_required_factor(0.5, 3.0) < 0.0);
    MT_ASSERT(close_enough(amdahl_required_factor(0.9, 5.0), 0.9 / (0.2 - 0.1)));
    MT_REPORT();
}
