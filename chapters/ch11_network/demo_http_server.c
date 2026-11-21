/* HTTP/1.0 server. With no arguments it serves one local GET of /index.html
 * and exits. Pass a port number to stay in the accept loop.
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
    addr.sin_port = htons((uint16_t)*port);
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

static int self_check(void) {
    int port = 0;
    int lfd;
    int status = 0;
    int http_status = 0;
    unsigned char *body = NULL;
    size_t blen = 0;
    int ok = 0;
    pid_t pid;

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
    if (http_fetch("127.0.0.1", port, "/index.html", &http_status, &body, &blen) == 0 &&
        http_status == 200 && body && strstr((char *)body, "systems-craft") != NULL) {
        ok = 1;
    }
    free(body);
    kill(pid, SIGTERM);
    while (waitpid(pid, &status, 0) < 0 && errno == EINTR) {
    }
    if (!ok) {
        fprintf(stderr, "index fetch failed\n");
        return 1;
    }
    printf("server-ok\n");
    return 0;
}

int main(int argc, char **argv) {
    int port;
    int lfd;

    arm_alarm();
    if (chdir_exe() < 0) {
        perror("chdir");
        return 1;
    }
    if (argc < 2) {
        return self_check();
    }
    alarm(0);
    port = atoi(argv[1]);
    if (port < 0 || port > 65535) {
        fprintf(stderr, "bad port\n");
        return 1;
    }
    lfd = listen_loopback(&port);
    if (lfd < 0) {
        perror("listen");
        return 1;
    }
    printf("http://127.0.0.1:%d\n", port);
    fflush(stdout);
    /* Stay up only when a port was requested. Alarm still caps a hang. */
    http_serve_fd(lfd);
    close(lfd);
    return 0;
}
