#include "sc_time.h"

#include <time.h>

double sc_now_sec(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
}

double sc_elapsed_sec(double start_sec) {
    return sc_now_sec() - start_sec;
}
