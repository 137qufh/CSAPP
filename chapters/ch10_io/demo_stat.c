/* Create a regular file and report its size and type from stat. */

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
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

int main(void) {
    char path[] = "/tmp/scraft-stat-XXXXXX";
    const char payload[] = "stat-sample";
    struct stat st;
    int fd = mkstemp(path);
    if (fd < 0) {
        perror("mkstemp");
        return 1;
    }
    if (write_full(fd, payload, sizeof(payload) - 1) < 0) {
        perror("write");
        close(fd);
        unlink(path);
        return 1;
    }
    if (close(fd) < 0) {
        perror("close");
        unlink(path);
        return 1;
    }
    if (stat(path, &st) < 0) {
        perror("stat");
        unlink(path);
        return 1;
    }
    printf("size=%lld regular=%d\n", (long long)st.st_size,
           S_ISREG(st.st_mode) ? 1 : 0);
    unlink(path);
    if (!S_ISREG(st.st_mode) || st.st_size != (off_t)(sizeof(payload) - 1)) {
        fprintf(stderr, "unexpected stat result\n");
        return 1;
    }
    return 0;
}
