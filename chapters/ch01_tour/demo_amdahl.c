/* Amdahl's law: if fraction p of the time is sped up by factor s,
 * overall speedup is 1 / ((1-p) + p/s).
 */
#include <stdio.h>
#include <stdlib.h>

static double speedup(double p, double s) {
    return 1.0 / ((1.0 - p) + p / s);
}

int main(int argc, char **argv) {
    double p = 0.6;
    double s = 10.0;
    if (argc == 3) {
        p = atof(argv[1]);
        s = atof(argv[2]);
    }
    if (p < 0 || p > 1 || s <= 0) {
        fprintf(stderr, "usage: %s <fraction 0..1> <speedup> \n", argv[0]);
        return 1;
    }
    printf("fraction=%.4f  factor=%.4f  overall=%.4f\n", p, s, speedup(p, s));
    printf("Even an infinite factor on that fraction tops out at %.4f\n",
           speedup(p, 1e12));
    return 0;
}
