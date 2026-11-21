#ifndef EX_LOCALITY_H
#define EX_LOCALITY_H

/* Sum every element of a row-major rows*cols matrix stored in a[].
 * row_sum changes the column fastest: index r*cols + c.
 * col_sum changes the row fastest and visits the same elements.
 * The two totals are equal. rows == 0 or cols == 0 yields 0.
 */
int row_sum(const int *a, int rows, int cols);
int col_sum(const int *a, int rows, int cols);

#endif
