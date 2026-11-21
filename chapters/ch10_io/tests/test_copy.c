#include "ex_copy.h"
#include "minitest.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int write_temp(char *path, const unsigned char *data, size_t n) {
    int fd = mkstemp(path);
    size_t off = 0;
    if (fd < 0) {
        return -1;
    }
    while (off < n) {
        ssize_t w = write(fd, data + off, n - off);
        if (w < 0) {
            if (errno == EINTR) {
                continue;
            }
            close(fd);
            return -1;
        }
        if (w == 0) {
            close(fd);
            return -1;
        }
        off += (size_t)w;
    }
    if (close(fd) < 0) {
        return -1;
    }
    return 0;
}

static int read_all(const char *path, unsigned char *buf, size_t cap, size_t *out) {
    int fd = open(path, O_RDONLY);
    size_t off = 0;
    if (fd < 0) {
        return -1;
    }
    while (off < cap) {
        ssize_t r = read(fd, buf + off, cap - off);
        if (r < 0) {
            if (errno == EINTR) {
                continue;
            }
            close(fd);
            return -1;
        }
        if (r == 0) {
            break;
        }
        off += (size_t)r;
    }
    close(fd);
    *out = off;
    return 0;
}

static void expect_copy(const unsigned char *data, size_t n) {
    char src[] = "/tmp/scraft-src-XXXXXX";
    char dst[] = "/tmp/scraft-dst-XXXXXX";
    int dstfd;
    unsigned char *got;
    size_t got_n = 0;
    int rc;

    dstfd = mkstemp(dst);
    if (dstfd < 0 || write_temp(src, data, n) < 0) {
        MT_ASSERT(0);
        return;
    }
    /* Pre-fill the destination so a successful copy must truncate it. */
    if (write(dstfd, "OLD!", 4) != 4) {
        MT_ASSERT(0);
        close(dstfd);
        unlink(src);
        unlink(dst);
        return;
    }
    close(dstfd);

    rc = copy_file(src, dst);
    MT_EQ_INT(rc, 0);
    if (rc == 0) {
        got = malloc(n + 8);
        if (!got || read_all(dst, got, n + 8, &got_n) < 0) {
            MT_ASSERT(0);
        } else {
            MT_EQ_INT((int)got_n, (int)n);
            MT_ASSERT(got_n == n && memcmp(got, data, n) == 0);
        }
        free(got);
    }
    unlink(src);
    unlink(dst);
}

int main(void) {
    unsigned char big[10000];
    size_t i;
    char dst[] = "/tmp/scraft-missing-dst-XXXXXX";
    int fd;

    {
        unsigned char empty_marker = 0;
        expect_copy(&empty_marker, 0);
    }

    for (i = 0; i < sizeof(big); i++) {
        big[i] = (unsigned char)(i * 17 + 3);
    }
    expect_copy(big, sizeof(big));

    fd = mkstemp(dst);
    if (fd < 0) {
        MT_ASSERT(0);
    } else {
        close(fd);
        MT_EQ_INT(copy_file("/tmp/scraft-no-such-copy-src", dst), -1);
        MT_EQ_INT(copy_file(NULL, dst), -1);
        unlink(dst);
    }
    MT_REPORT();
}
