/* Start the server on an ephemeral port and check a static page plus CGI. */

#include "http_client.h"
#include "http_server.h"

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
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

static int chdir_exe(void) {
    char path[4096];
    ssize_t n = readlink("/proc/self/exe", path, sizeof(path) - 1);
    char *slash;
    if (n < 0) {
        return -1;
    }
    path[n] = '\0';
    slash = strrchr(path, '/');
    if (!slash) {
        return -1;
    }
    *slash = '\0';
    return chdir(path);
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
    if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0 || listen(fd, 64) < 0) {
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

static int body_has(const unsigned char *body, const char *needle) {
    return body && strstr((const char *)body, needle) != NULL;
}

int main(void) {
    int port = 0;
    int lfd;
    int status = 0;
    int http_status = 0;
    unsigned char *body = NULL;
    size_t blen = 0;
    int ok = 1;
    pid_t pid;

    arm_alarm();
    if (chdir_exe() < 0) {
        perror("chdir");
        return 1;
    }
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
        alarm(5);
        http_serve_fd(lfd);
        _exit(0);
    }
    close(lfd);

    if (http_fetch("127.0.0.1", port, "/index.html", &http_status, &body, &blen) != 0 ||
        http_status != 200 || !body_has(body, "systems-craft")) {
        fprintf(stderr, "static page missing systems-craft\n");
        ok = 0;
    }
    free(body);
    body = NULL;

    http_status = 0;
    blen = 0;
    if (ok && (http_fetch("127.0.0.1", port, "/cgi/hello?name=lin", &http_status, &body,
                          &blen) != 0 ||
               http_status != 200 || !body_has(body, "hello, lin"))) {
        fprintf(stderr, "cgi greeting missing\n");
        ok = 0;
    }
    free(body);

    kill(pid, SIGTERM);
    while (waitpid(pid, &status, 0) < 0 && errno == EINTR) {
    }
    if (!ok) {
        return 1;
    }
    printf("http-ok\n");
    return 0;
}
