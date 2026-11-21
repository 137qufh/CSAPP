#ifndef CACHECRAFT_TRANSPOSE_H
#define CACHECRAFT_TRANSPOSE_H

/* Row-major n by n transpose. src and dst must not overlap.
 * dst[j * n + i] = src[i * n + j].
 */
void transpose(int n, int *src, int *dst);

#endif
