/* Echo server that forks one worker per connection and reaps it with SIGCHLD.
 * The original process is the client on 127.0.0.1. No threads in this file.
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

static volatile sig_atomic_t reaped = 0;

static void on_sigchld(int sig) {
    int status;
    (void)sig;
    while (waitpid(-1, &status, WNOHANG) > 0) {
        reaped++;
    }
}

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
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGPIPE, &sa, NULL);
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

static void serve_one(int lfd) {
    sigset_t block;
    sigset_t old;
    struct sigaction sa;
    struct sockaddr_in cli;
    socklen_t len = sizeof(cli);
    int client;
    pid_t worker;

    alarm(5);
    sigemptyset(&block);
    sigaddset(&block, SIGCHLD);
    sigprocmask(SIG_BLOCK, &block, &old);

    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = on_sigchld;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGCHLD, &sa, NULL);

    client = accept(lfd, (struct sockaddr *)&cli, &len);
    if (client < 0) {
        _exit(1);
    }
    worker = fork();
    if (worker < 0) {
        _exit(1);
    }
    if (worker == 0) {
        char line[64];
        int got;
        sigprocmask(SIG_SETMASK, &old, NULL);
        close(lfd);
        tune_timeout(client);
        got = read_line(client, line, sizeof(line));
        if (got > 0 && write_full(client, line, (size_t)got) < 0) {
            close(client);
            _exit(1);
        }
        close(client);
        _exit(0);
    }
    close(client);
    while (reaped == 0) {
        sigsuspend(&old);
    }
    sigprocmask(SIG_SETMASK, &old, NULL);
    close(lfd);
    _exit(0);
}

int main(void) {
    int port = 0;
    int lfd;
    int cfd;
    int n;
    int status = 0;
    char line[64];
    pid_t pid;
    const char msg[] = "proc-ping\n";

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
        serve_one(lfd);
    }
    close(lfd);
    cfd = connect_loopback(port);
    if (cfd < 0 || write_full(cfd, msg, sizeof(msg) - 1) < 0) {
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
    if (n < 0 || strcmp(line, msg) != 0 || !WIFEXITED(status) ||
        WEXITSTATUS(status) != 0) {
        fprintf(stderr, "proc echo failed: [%s]\n", line);
        return 1;
    }
    printf("proc-ok\n");
    return 0;
}
