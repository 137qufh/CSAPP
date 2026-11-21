/* Fetch one URL with our own sockets.
 * No argument: ask the local server for /index.html and print the body.
 * One argument: http://127.0.0.1:PORT/path
 */

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

static int parse_url(const char *url, char *host, size_t host_cap, int *port,
                     char *path, size_t path_cap) {
    const char *p;
    const char *colon;
    const char *slash;
    size_t hlen;
    size_t plen;
    if (strncmp(url, "http://", 7) != 0) {
        return -1;
    }
    p = url + 7;
    colon = strchr(p, ':');
    slash = strchr(p, '/');
    if (!colon || !slash || colon > slash) {
        return -1;
    }
    hlen = (size_t)(colon - p);
    plen = strlen(slash);
    if (hlen + 1 > host_cap || plen + 1 > path_cap) {
        return -1;
    }
    memcpy(host, p, hlen);
    host[hlen] = '\0';
    *port = atoi(colon + 1);
    if (*port <= 0 || *port > 65535) {
        return -1;
    }
    memcpy(path, slash, plen + 1);
    return 0;
}

static int fetch_and_print(const char *host, int port, const char *path) {
    int status = 0;
    unsigned char *body = NULL;
    size_t blen = 0;
    if (http_fetch(host, port, path, &status, &body, &blen) < 0) {
        fprintf(stderr, "fetch failed\n");
        return 1;
    }
    if (fwrite(body, 1, blen, stdout) != blen) {
        free(body);
        return 1;
    }
    if (blen == 0 || body[blen - 1] != '\n') {
        if (fputc('\n', stdout) == EOF) {
            free(body);
            return 1;
        }
    }
    free(body);
    return status == 200 ? 0 : 1;
}

static int self_check(void) {
    int port = 0;
    int lfd;
    int status = 0;
    pid_t pid;
    int rc;

    lfd = listen_loopback(&port);
    if (lfd < 0) {
        perror("listen");
        return 1;
    }
    fflush(NULL);
    pid = fork();
    if (pid < 0) {
        close(lfd);
        return 1;
    }
    if (pid == 0) {
        alarm(5);
        http_serve_fd(lfd);
        _exit(0);
    }
    close(lfd);
    rc = fetch_and_print("127.0.0.1", port, "/index.html");
    kill(pid, SIGTERM);
    while (waitpid(pid, &status, 0) < 0 && errno == EINTR) {
    }
    return rc;
}

int main(int argc, char **argv) {
    char host[128];
    char path[768];
    int port = 0;

    arm_alarm();
    if (chdir_exe() < 0) {
        perror("chdir");
        return 1;
    }
    if (argc < 2) {
        return self_check();
    }
    if (parse_url(argv[1], host, sizeof(host), &port, path, sizeof(path)) < 0) {
        fprintf(stderr, "usage: %s http://127.0.0.1:PORT/path\n", argv[0]);
        return 1;
    }
    return fetch_and_print(host, port, path);
}
