/* epoll watches a loopback listener, then the accepted socket.
 * One client connects, sends a short payload, and reads it back.
 */

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/time.h>
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

static int ep_add(int ep, int fd) {
    struct epoll_event ev;
    memset(&ev, 0, sizeof(ev));
    ev.events = EPOLLIN;
    ev.data.fd = fd;
    return epoll_ctl(ep, EPOLL_CTL_ADD, fd, &ev);
}

static int ep_wait_fd(int ep, int want) {
    for (;;) {
        struct epoll_event out;
        int n = epoll_wait(ep, &out, 1, 2000);
        if (n < 0) {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }
        if (n != 1 || out.data.fd != want) {
            return -1;
        }
        return 0;
    }
}

int main(void) {
    const char msg[] = "epoll-ping";
    const size_t msg_len = sizeof(msg) - 1;
    int port = 0;
    int lfd;
    int ep;
    int cfd;
    int conn = -1;
    char buf[64];
    char back[64];
    size_t got = 0;
    size_t ngot = 0;
    struct sockaddr_in cli;
    socklen_t len;

    arm_alarm();
    lfd = listen_loopback(&port);
    if (lfd < 0) {
        perror("listen");
        return 1;
    }
    ep = epoll_create1(0);
    if (ep < 0 || ep_add(ep, lfd) < 0) {
        perror("epoll");
        close(lfd);
        if (ep >= 0) {
            close(ep);
        }
        return 1;
    }
    cfd = connect_loopback(port);
    if (cfd < 0 || write_full(cfd, msg, msg_len) < 0) {
        perror("client");
        close(lfd);
        close(ep);
        if (cfd >= 0) {
            close(cfd);
        }
        return 1;
    }
    if (ep_wait_fd(ep, lfd) < 0) {
        fprintf(stderr, "epoll did not report the listener\n");
        close(cfd);
        close(lfd);
        close(ep);
        return 1;
    }
    len = sizeof(cli);
    conn = accept(lfd, (struct sockaddr *)&cli, &len);
    if (conn < 0 || ep_add(ep, conn) < 0) {
        perror("accept");
        close(cfd);
        close(lfd);
        close(ep);
        if (conn >= 0) {
            close(conn);
        }
        return 1;
    }
    while (got < msg_len) {
        ssize_t r;
        if (ep_wait_fd(ep, conn) < 0) {
            fprintf(stderr, "epoll did not report the connection\n");
            close(conn);
            close(cfd);
            close(lfd);
            close(ep);
            return 1;
        }
        r = read(conn, buf + got, sizeof(buf) - got);
        if (r < 0) {
            if (errno == EINTR) {
                continue;
            }
            perror("read");
            close(conn);
            close(cfd);
            close(lfd);
            close(ep);
            return 1;
        }
        if (r == 0) {
            break;
        }
        got += (size_t)r;
    }
    if (got != msg_len || memcmp(buf, msg, msg_len) != 0 ||
        write_full(conn, buf, got) < 0) {
        fprintf(stderr, "epoll payload mismatch\n");
        close(conn);
        close(cfd);
        close(lfd);
        close(ep);
        return 1;
    }
    while (ngot < got) {
        ssize_t r = read(cfd, back + ngot, got - ngot);
        if (r < 0) {
            if (errno == EINTR) {
                continue;
            }
            perror("client read");
            close(conn);
            close(cfd);
            close(lfd);
            close(ep);
            return 1;
        }
        if (r == 0) {
            break;
        }
        ngot += (size_t)r;
    }
    close(conn);
    close(cfd);
    close(lfd);
    close(ep);
    if (ngot != got || memcmp(back, msg, got) != 0) {
        fprintf(stderr, "client recv mismatch\n");
        return 1;
    }
    printf("epoll-ok\n");
    return 0;
}
