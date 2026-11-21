/* Write the same text through stdio and through write(), then compare counts.
 * fflush pushes the user-space FILE buffer into the kernel. fsync asks the
 * kernel to commit those bytes to the underlying device. They are different
 * layers: fflush does not imply fsync, and write() does not flush any FILE.
 */

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

int main(void) {
    const char text[] = "stdio and write should agree\n";
    const size_t len = sizeof(text) - 1;
    char path_std[] = "/tmp/scraft-stdio-XXXXXX";
    char path_unix[] = "/tmp/scraft-unix-XXXXXX";
    FILE *fp;
    int fd;
    size_t nstdio;
    int nunix_ok;

    fd = mkstemp(path_std);
    if (fd < 0) {
        perror("mkstemp");
        return 1;
    }
    fp = fdopen(fd, "w");
    if (!fp) {
        perror("fdopen");
        close(fd);
        unlink(path_std);
        return 1;
    }
    nstdio = fwrite(text, 1, len, fp);
    if (fflush(fp) != 0 || fsync(fileno(fp)) != 0) {
        perror("flush stdio");
        fclose(fp);
        unlink(path_std);
        return 1;
    }
    if (fclose(fp) != 0) {
        perror("fclose");
        unlink(path_std);
        return 1;
    }

    fd = mkstemp(path_unix);
    if (fd < 0) {
        perror("mkstemp");
        unlink(path_std);
        return 1;
    }
    nunix_ok = (write_full(fd, text, len) == 0);
    if (!nunix_ok || fsync(fd) != 0) {
        perror("write");
        close(fd);
        unlink(path_std);
        unlink(path_unix);
        return 1;
    }
    if (close(fd) < 0) {
        perror("close");
        unlink(path_std);
        unlink(path_unix);
        return 1;
    }

    printf("fwrite=%zu write=%zu\n", nstdio, len);
    unlink(path_std);
    unlink(path_unix);
    if (nstdio != len) {
        fprintf(stderr, "byte counts differ\n");
        return 1;
    }
    return 0;
}
