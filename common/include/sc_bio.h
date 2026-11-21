#ifndef SC_BIO_H
#define SC_BIO_H

#include <stddef.h>
#include <sys/types.h>

/* Buffered I/O for unreliable short counts.
 * Unbuffered helpers retry until n bytes move or EOF/error.
 * The buffered reader keeps an internal window for line-oriented input.
 */

#define SC_BIO_BUF 8192

typedef struct sc_bio {
    int fd;
    int nleft;          /* unread bytes sitting in buf */
    char *cursor;       /* next unread byte */
    char buf[SC_BIO_BUF];
} sc_bio_t;

void sc_bio_init(sc_bio_t *b, int fd);

/* Retrying wrappers. Return bytes transferred, 0 on EOF, -1 on error. */
ssize_t sc_readn(int fd, void *buf, size_t n);
ssize_t sc_writen(int fd, const void *buf, size_t n);

/* Buffered. readline keeps the trailing newline when it fits. */
ssize_t sc_bio_read(sc_bio_t *b, void *buf, size_t n);
ssize_t sc_bio_readline(sc_bio_t *b, void *buf, size_t maxlen);

#endif
