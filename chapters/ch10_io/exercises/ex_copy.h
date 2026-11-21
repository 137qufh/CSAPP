#ifndef EX_COPY_H
#define EX_COPY_H

/* Copy all bytes from src path to dst path using read/write loops that
 * tolerate short counts. Return 0 on success, -1 on error. */
int copy_file(const char *src, const char *dst);

#endif
