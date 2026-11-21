/* One-shot echo on 127.0.0.1. The child accepts a single line; the parent
 * sends "ping" and checks the same line comes back. Port 0 picks a free port.
 */

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
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

static void tune_timeout(int fd) {
    struct timeval tv;
    tv.tv_sec = 2;
    tv.tv_usec = 0;
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
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

static int read_line(int fd, char *buf, size_t cap) {
    size_t i = 0;
    if (cap == 0) {
        return -1;
    }
    while (i + 1 < cap) {
        char c;
        ssize_t r = read(fd, &c, 1);
        if (r < 0) {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }
        if (r == 0) {
            break;
        }
        buf[i++] = c;
        if (c == '\n') {
            break;
        }
    }
    buf[i] = '\0';
    return (int)i;
}

static int listen_loopback(int *port) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    int yes = 1;
    struct sockaddr_in addr;
    socklen_t len;
    if (fd < 0) {
        return -1;
    }
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = htons(0);
    if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0 || listen(fd, 16) < 0) {
        close(fd);
        return -1;
    }
    len = sizeof(addr);
    if (getsockname(fd, (struct sockaddr *)&addr, &len) < 0) {
        close(fd);
        return -1;
    }
    *port = ntohs(addr.sin_port);
    return fd;
}

static int connect_loopback(int port) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    if (fd < 0) {
        return -1;
    }
    tune_timeout(fd);
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons((uint16_t)port);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        close(fd);
        return -1;
    }
    return fd;
}

int main(void) {
    int port = 0;
    int lfd;
    int cfd;
    int n;
    int status = 0;
    char line[64];
    pid_t pid;
    const char msg[] = "ping\n";

    arm_alarm();
    lfd = listen_loopback(&port);
    if (lfd < 0) {
        perror("listen");
        return 1;
    }
    fflush(NULL);
    pid = fork();
    if (pid < 0) {
        perror("fork");
        close(lfd);
        return 1;
    }
    if (pid == 0) {
        struct sockaddr_in cli;
        socklen_t len = sizeof(cli);
        int client;
        int got;
        alarm(5);
        client = accept(lfd, (struct sockaddr *)&cli, &len);
        close(lfd);
        if (client < 0) {
            _exit(1);
        }
        tune_timeout(client);
        got = read_line(client, line, sizeof(line));
        if (got > 0 && write_full(client, line, (size_t)got) < 0) {
            close(client);
            _exit(1);
        }
        close(client);
        _exit(0);
    }

    close(lfd);
    cfd = connect_loopback(port);
    if (cfd < 0 || write_full(cfd, msg, sizeof(msg) - 1) < 0) {
        perror("client");
        if (cfd >= 0) {
            close(cfd);
        }
        kill(pid, SIGTERM);
        waitpid(pid, &status, 0);
        return 1;
    }
    n = read_line(cfd, line, sizeof(line));
    close(cfd);
    if (waitpid(pid, &status, 0) < 0) {
        perror("waitpid");
        return 1;
    }
    if (n < 0 || strcmp(line, "ping\n") != 0) {
        fprintf(stderr, "echo mismatch: [%s]\n", line);
        return 1;
    }
    printf("echo-ok\n");
    return 0;
}
