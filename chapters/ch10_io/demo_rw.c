/* Write a buffer into a temporary file and read the same bytes back. */

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int write_full(int fd, const void *buf, size_t n) {
    const unsigned char *p = buf;
    size_t off = 0;
    while (off < n) {
        ssize_t w = write(fd, p + off, n - off);
        if (w < 0) {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }
        if (w == 0) {
            return -1;
        }
        off += (size_t)w;
    }
    return 0;
}

static int read_full(int fd, void *buf, size_t n) {
    unsigned char *p = buf;
    size_t off = 0;
    while (off < n) {
        ssize_t r = read(fd, p + off, n - off);
        if (r < 0) {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }
        if (r == 0) {
            return -1;
        }
        off += (size_t)r;
    }
    return 0;
}

int main(void) {
    char path[] = "/tmp/scraft-rw-XXXXXX";
    int fd = mkstemp(path);
    if (fd < 0) {
        perror("mkstemp");
        return 1;
    }

    const char payload[] = "craft-read-write";
    const size_t len = sizeof(payload) - 1;
    char back[sizeof(payload)];

    if (write_full(fd, payload, len) < 0 || lseek(fd, 0, SEEK_SET) < 0 ||
        read_full(fd, back, len) < 0) {
        perror("rw");
        close(fd);
        unlink(path);
        return 1;
    }
    close(fd);
    unlink(path);

    if (memcmp(back, payload, len) != 0) {
        fprintf(stderr, "payload mismatch\n");
        return 1;
    }
    printf("rw-ok\n");
    return 0;
}
