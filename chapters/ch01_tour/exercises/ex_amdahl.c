#include "ex_amdahl.h"

double amdahl_speedup(double improved_fraction, double speedup_factor) {
    (void)improved_fraction;
    (void)speedup_factor;
    /* TODO: return 1 / ((1-p) + p/s). */
    return 0.0;
}

double amdahl_required_factor(double improved_fraction, double target_speedup) {
    (void)improved_fraction;
    (void)target_speedup;
    /* TODO: solve Amdahl's law for s. If target is unreachable, return -1. */
    return 0.0;
}
