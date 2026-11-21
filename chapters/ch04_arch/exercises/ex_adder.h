#ifndef EX_ADDER_H
#define EX_ADDER_H

/* Build these from AND/OR/NOT only. Inputs are 0 or 1 unless noted.
 * add4 adds the low 4 bits and returns the low 4 bits of the sum.
 */
int gate_xor(int a, int b);
int gate_mux(int sel, int a, int b);
int ripple_add4(int a, int b);

#endif
