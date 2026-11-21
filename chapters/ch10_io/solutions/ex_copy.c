#include "ex_copy.h"

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

int copy_file(const char *src, const char *dst) {
    char buf[4096];
    int in;
    int out;
    if (!src || !dst) {
        return -1;
    }
    in = open(src, O_RDONLY);
    if (in < 0) {
        return -1;
    }
    out = open(dst, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (out < 0) {
        close(in);
        return -1;
    }
    for (;;) {
        ssize_t r = read(in, buf, sizeof(buf));
        size_t off = 0;
        if (r < 0) {
            if (errno == EINTR) {
                continue;
            }
            close(in);
            close(out);
            return -1;
        }
        if (r == 0) {
            break;
        }
        while (off < (size_t)r) {
            ssize_t w = write(out, buf + off, (size_t)r - off);
            if (w < 0) {
                if (errno == EINTR) {
                    continue;
                }
                close(in);
                close(out);
                return -1;
            }
            if (w == 0) {
                close(in);
                close(out);
                return -1;
            }
            off += (size_t)w;
        }
    }
    if (close(in) < 0) {
        close(out);
        return -1;
    }
    if (close(out) < 0) {
        return -1;
    }
    return 0;
}
