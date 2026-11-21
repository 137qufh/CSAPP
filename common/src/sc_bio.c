#include "sc_bio.h"

#include <errno.h>
#include <string.h>
#include <unistd.h>

void sc_bio_init(sc_bio_t *b, int fd) {
    b->fd = fd;
    b->nleft = 0;
    b->cursor = b->buf;
}

ssize_t sc_readn(int fd, void *buf, size_t n) {
    size_t got = 0;
    unsigned char *p = buf;
    while (got < n) {
        ssize_t r = read(fd, p + got, n - got);
        if (r == 0) {
            return (ssize_t)got;
        }
        if (r < 0) {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }
        got += (size_t)r;
    }
    return (ssize_t)got;
}

ssize_t sc_writen(int fd, const void *buf, size_t n) {
    size_t put = 0;
    const unsigned char *p = buf;
    while (put < n) {
        ssize_t r = write(fd, p + put, n - put);
        if (r < 0) {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }
        put += (size_t)r;
    }
    return (ssize_t)put;
}

static ssize_t bio_refill(sc_bio_t *b) {
    for (;;) {
        ssize_t r = read(b->fd, b->buf, SC_BIO_BUF);
        if (r < 0) {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }
        b->nleft = (int)r;
        b->cursor = b->buf;
        return r;
    }
}

ssize_t sc_bio_read(sc_bio_t *b, void *buf, size_t n) {
    size_t got = 0;
    unsigned char *p = buf;
    while (got < n) {
        if (b->nleft <= 0) {
            ssize_t r = bio_refill(b);
            if (r < 0) {
                return -1;
            }
            if (r == 0) {
                return (ssize_t)got;
            }
        }
        size_t chunk = n - got;
        if ((int)chunk > b->nleft) {
            chunk = (size_t)b->nleft;
        }
        memcpy(p + got, b->cursor, chunk);
        b->cursor += chunk;
        b->nleft -= (int)chunk;
        got += chunk;
    }
    return (ssize_t)got;
}

ssize_t sc_bio_readline(sc_bio_t *b, void *buf, size_t maxlen) {
    size_t i = 0;
    char *out = buf;
    if (maxlen == 0) {
        return 0;
    }
    while (i + 1 < maxlen) {
        if (b->nleft <= 0) {
            ssize_t r = bio_refill(b);
            if (r < 0) {
                return -1;
            }
            if (r == 0) {
                break;
            }
        }
        char c = *b->cursor++;
        b->nleft--;
        out[i++] = c;
        if (c == '\n') {
            break;
        }
    }
    out[i] = '\0';
    return (ssize_t)i;
}
