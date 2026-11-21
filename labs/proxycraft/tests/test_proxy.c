#include "minitest.h"

#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>

static pid_t spawn(const char *bin, int *port) {
    int fds[2];
    if (pipe(fds) < 0) {
        return -1;
    }
    pid_t pid = fork();
    if (pid < 0) {
        return -1;
    }
    if (pid == 0) {
        dup2(fds[1], STDOUT_FILENO);
        close(fds[0]);
        close(fds[1]);
        execl(bin, bin, (char *)NULL);
        _exit(127);
    }
    close(fds[1]);
    char buf[64] = {0};
    size_t n = 0;
    while (n + 1 < sizeof(buf)) {
        char c;
        ssize_t r = read(fds[0], &c, 1);
        if (r <= 0) {
            break;
        }
        buf[n++] = c;
        if (c == '\n') {
            break;
        }
    }
    close(fds[0]);
    int p = 0;
    if (sscanf(buf, "port %d", &p) != 1) {
        kill(pid, SIGKILL);
        waitpid(pid, NULL, 0);
        return -1;
    }
    *port = p;
    return pid;
}

static int http_get(int port, const char *req, char *out, size_t outn) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        return -1;
    }
    struct sockaddr_in a;
    memset(&a, 0, sizeof(a));
    a.sin_family = AF_INET;
    a.sin_port = htons((uint16_t)port);
    inet_pton(AF_INET, "127.0.0.1", &a.sin_addr);
    if (connect(fd, (struct sockaddr *)&a, sizeof(a)) < 0) {
        close(fd);
        return -1;
    }
    size_t len = strlen(req);
    if (write(fd, req, len) != (ssize_t)len) {
        close(fd);
        return -1;
    }
    size_t n = 0;
    while (n + 1 < outn) {
        ssize_t r = read(fd, out + n, outn - 1 - n);
        if (r < 0) {
            if (errno == EINTR) {
                continue;
            }
            break;
        }
        if (r == 0) {
            break;
        }
        n += (size_t)r;
    }
    out[n] = '\0';
    close(fd);
    return (int)n;
}

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "usage: test_proxy origin proxy\n");
        return 2;
    }
    alarm(10);
    int oport = 0, pport = 0;
    pid_t op = spawn(argv[1], &oport);
    pid_t pp = spawn(argv[2], &pport);
    MT_ASSERT(op > 0);
    MT_ASSERT(pp > 0);
    char req[256], resp[1024];
    snprintf(req, sizeof(req), "GET http://127.0.0.1:%d/a HTTP/1.0\r\nHost: x\r\n\r\n", oport);
    MT_ASSERT(http_get(pport, req, resp, sizeof(resp)) > 0);
    MT_ASSERT(strstr(resp, "page-a") != NULL);
    MT_ASSERT(http_get(pport, req, resp, sizeof(resp)) > 0);
    MT_ASSERT(strstr(resp, "page-a") != NULL);

    char direct[128];
    snprintf(direct, sizeof(direct), "GET /count HTTP/1.0\r\nHost: x\r\n\r\n");
    MT_ASSERT(http_get(oport, direct, resp, sizeof(resp)) > 0);
    MT_ASSERT(strstr(resp, "count=1") != NULL);

    snprintf(req, sizeof(req), "GET http://127.0.0.1:%d/b HTTP/1.0\r\nHost: x\r\n\r\n", oport);
    MT_ASSERT(http_get(pport, req, resp, sizeof(resp)) > 0);
    MT_ASSERT(strstr(resp, "page-b") != NULL);
    MT_ASSERT(http_get(oport, direct, resp, sizeof(resp)) > 0);
    MT_ASSERT(strstr(resp, "count=2") != NULL);

    kill(pp, SIGTERM);
    kill(op, SIGTERM);
    waitpid(pp, NULL, 0);
    waitpid(op, NULL, 0);
    MT_REPORT();
}
