#ifndef EX_AMDAHL_H
#define EX_AMDAHL_H

/* p is the fraction of original time improved by factor s (s > 0). */
double amdahl_speedup(double improved_fraction, double speedup_factor);

/* How many times faster the improved part must be to hit `target` overall.
 * Return a negative number if the target is impossible.
 */
double amdahl_required_factor(double improved_fraction, double target_speedup);

#endif
