/* Read two pipes with select. Children write one line each and close.
 * The parent waits on readiness instead of sleeping.
 */

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <unistd.h>

static void on_alarm(int sig) {
    const char msg[] = "timeout\n";
    ssize_t w;
    (void)sig;
    w = write(STDERR_FILENO, msg, sizeof(msg) - 1);
    (void)w;
    _exit(2);
}

static void arm_alarm(void) {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = on_alarm;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGALRM, &sa, NULL);
    alarm(5);
}

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

static int set_nonblock(int fd) {
    int flags = fcntl(fd, F_GETFL);
    if (flags < 0) {
        return -1;
    }
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

static int pull(int fd, char *buf, size_t cap, size_t *len, int *open_flag) {
    while (*len + 1 < cap) {
        ssize_t n = read(fd, buf + *len, cap - 1 - *len);
        if (n < 0) {
            if (errno == EINTR) {
                continue;
            }
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                return 0;
            }
            return -1;
        }
        if (n == 0) {
            *open_flag = 0;
            buf[*len] = '\0';
            return 0;
        }
        *len += (size_t)n;
        buf[*len] = '\0';
    }
    return -1;
}

int main(void) {
    int pa[2];
    int pb[2];
    pid_t ca;
    pid_t cb;
    char got_a[32];
    char got_b[32];
    size_t len_a = 0;
    size_t len_b = 0;
    int open_a = 1;
    int open_b = 1;
    int status = 0;

    arm_alarm();
    if (pipe(pa) < 0 || pipe(pb) < 0) {
        perror("pipe");
        return 1;
    }
    fflush(NULL);
    ca = fork();
    if (ca < 0) {
        return 1;
    }
    if (ca == 0) {
        close(pa[0]);
        close(pb[0]);
        close(pb[1]);
        if (write_full(pa[1], "alpha\n", 6) < 0) {
            _exit(1);
        }
        close(pa[1]);
        _exit(0);
    }
    cb = fork();
    if (cb < 0) {
        return 1;
    }
    if (cb == 0) {
        close(pb[0]);
        close(pa[0]);
        close(pa[1]);
        if (write_full(pb[1], "beta\n", 5) < 0) {
            _exit(1);
        }
        close(pb[1]);
        _exit(0);
    }
    close(pa[1]);
    close(pb[1]);
    if (set_nonblock(pa[0]) < 0 || set_nonblock(pb[0]) < 0) {
        return 1;
    }
    got_a[0] = '\0';
    got_b[0] = '\0';

    while (open_a || open_b) {
        fd_set rfds;
        struct timeval tv;
        int maxfd = 0;
        int rc;
        FD_ZERO(&rfds);
        if (open_a) {
            FD_SET(pa[0], &rfds);
            if (pa[0] > maxfd) {
                maxfd = pa[0];
            }
        }
        if (open_b) {
            FD_SET(pb[0], &rfds);
            if (pb[0] > maxfd) {
                maxfd = pb[0];
            }
        }
        tv.tv_sec = 2;
        tv.tv_usec = 0;
        rc = select(maxfd + 1, &rfds, NULL, NULL, &tv);
        if (rc < 0) {
            if (errno == EINTR) {
                continue;
            }
            perror("select");
            return 1;
        }
        if (rc == 0) {
            fprintf(stderr, "select timed out\n");
            return 1;
        }
        if (open_a && FD_ISSET(pa[0], &rfds)) {
            if (pull(pa[0], got_a, sizeof(got_a), &len_a, &open_a) < 0) {
                return 1;
            }
        }
        if (open_b && FD_ISSET(pb[0], &rfds)) {
            if (pull(pb[0], got_b, sizeof(got_b), &len_b, &open_b) < 0) {
                return 1;
            }
        }
    }
    close(pa[0]);
    close(pb[0]);
    if (waitpid(ca, &status, 0) < 0 || waitpid(cb, &status, 0) < 0) {
        perror("waitpid");
        return 1;
    }
    printf("%s%s", got_a, got_b);
    if (strcmp(got_a, "alpha\n") != 0 || strcmp(got_b, "beta\n") != 0) {
        fprintf(stderr, "select data mismatch\n");
        return 1;
    }
    printf("select-ok\n");
    return 0;
}
