#ifndef SC_TIME_H
#define SC_TIME_H

/* Monotonic timing helpers. Cycles-per-element style measurements live in ch05;
 * this file only provides a stable nanosecond clock.
 */

double sc_now_sec(void);
double sc_elapsed_sec(double start_sec);

#endif
