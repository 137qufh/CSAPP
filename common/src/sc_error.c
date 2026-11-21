#include "sc_error.h"

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>

static void sc_vwarn(const char *tag, const char *fmt, va_list ap) {
    fprintf(stderr, "%s: ", tag);
    vfprintf(stderr, fmt, ap);
    if (errno) {
        fprintf(stderr, ": %s", strerror(errno));
    }
    fputc('\n', stderr);
}

void sc_app_error(const char *fmt, ...) {
    va_list ap;
    int saved = errno;
    errno = 0;
    va_start(ap, fmt);
    sc_vwarn("error", fmt, ap);
    va_end(ap);
    errno = saved;
    exit(1);
}

void sc_unix_error(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    sc_vwarn("unix", fmt, ap);
    va_end(ap);
    exit(1);
}

void *sc_malloc(size_t size) {
    void *p = malloc(size ? size : 1);
    if (!p) {
        sc_unix_error("malloc(%zu)", size);
    }
    return p;
}

void *sc_calloc(size_t n, size_t size) {
    void *p = calloc(n ? n : 1, size ? size : 1);
    if (!p) {
        sc_unix_error("calloc(%zu,%zu)", n, size);
    }
    return p;
}

void *sc_realloc(void *ptr, size_t size) {
    void *p = realloc(ptr, size ? size : 1);
    if (!p) {
        sc_unix_error("realloc(%zu)", size);
    }
    return p;
}

int sc_open(const char *path, int flags, mode_t mode) {
    int fd = open(path, flags, mode);
    if (fd < 0) {
        sc_unix_error("open(%s)", path);
    }
    return fd;
}

void sc_close(int fd) {
    if (close(fd) < 0) {
        sc_unix_error("close");
    }
}

ssize_t sc_read(int fd, void *buf, size_t n) {
    ssize_t r = read(fd, buf, n);
    if (r < 0) {
        sc_unix_error("read");
    }
    return r;
}

ssize_t sc_write(int fd, const void *buf, size_t n) {
    ssize_t r = write(fd, buf, n);
    if (r < 0) {
        sc_unix_error("write");
    }
    return r;
}

void sc_dup2(int oldfd, int newfd) {
    if (dup2(oldfd, newfd) < 0) {
        sc_unix_error("dup2");
    }
}

pid_t sc_fork(void) {
    pid_t pid = fork();
    if (pid < 0) {
        sc_unix_error("fork");
    }
    return pid;
}

pid_t sc_waitpid(pid_t pid, int *status, int options) {
    pid_t r = waitpid(pid, status, options);
    if (r < 0 && errno != ECHILD) {
        sc_unix_error("waitpid");
    }
    return r;
}

void sc_execve(const char *path, char *const argv[], char *const envp[]) {
    execve(path, argv, envp);
    sc_unix_error("execve(%s)", path);
}

void sc_kill(pid_t pid, int sig) {
    if (kill(pid, sig) < 0) {
        sc_unix_error("kill");
    }
}

int sc_socket(int domain, int type, int protocol) {
    int fd = socket(domain, type, protocol);
    if (fd < 0) {
        sc_unix_error("socket");
    }
    return fd;
}

void sc_bind(int fd, const struct sockaddr *addr, socklen_t len) {
    if (bind(fd, addr, len) < 0) {
        sc_unix_error("bind");
    }
}

void sc_listen(int fd, int backlog) {
    if (listen(fd, backlog) < 0) {
        sc_unix_error("listen");
    }
}

int sc_accept(int fd, struct sockaddr *addr, socklen_t *len) {
    int c = accept(fd, addr, len);
    if (c < 0) {
        sc_unix_error("accept");
    }
    return c;
}

void sc_connect(int fd, const struct sockaddr *addr, socklen_t len) {
    if (connect(fd, addr, len) < 0) {
        sc_unix_error("connect");
    }
}

int sc_listen_local(int *port_out) {
    int fd = sc_socket(AF_INET, SOCK_STREAM, 0);
    int yes = 1;
    struct sockaddr_in addr;
    socklen_t len;

    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = htons(0);
    sc_bind(fd, (struct sockaddr *)&addr, sizeof(addr));
    sc_listen(fd, 64);
    len = sizeof(addr);
    if (getsockname(fd, (struct sockaddr *)&addr, &len) < 0) {
        sc_unix_error("getsockname");
    }
    if (port_out) {
        *port_out = ntohs(addr.sin_port);
    }
    return fd;
}
