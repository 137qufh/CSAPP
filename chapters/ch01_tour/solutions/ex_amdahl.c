#include "ex_amdahl.h"

double amdahl_speedup(double improved_fraction, double speedup_factor) {
    double p = improved_fraction;
    double s = speedup_factor;
    return 1.0 / ((1.0 - p) + p / s);
}

double amdahl_required_factor(double improved_fraction, double target_speedup) {
    double p = improved_fraction;
    double t = target_speedup;
    /* t = 1 / ((1-p) + p/s)  =>  (1-p) + p/s = 1/t  =>  p/s = 1/t - (1-p)
     * s = p / (1/t - (1-p))
     * Impossible when the denominator is <= 0, i.e. t >= 1/(1-p) for p<1.
     */
    if (p <= 0.0) {
        return (t <= 1.0) ? 1.0 : -1.0;
    }
    double denom = (1.0 / t) - (1.0 - p);
    if (denom <= 0.0) {
        return -1.0;
    }
    return p / denom;
}
