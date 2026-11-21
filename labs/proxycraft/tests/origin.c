#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

/* Tiny origin. GET /a and /b increment a counter. GET /count reports it. */

static int hits;

static void serve(int fd) {
    char buf[1024];
    size_t n = 0;
    while (n + 1 < sizeof(buf)) {
        ssize_t r = read(fd, buf + n, 1);
        if (r <= 0) {
            break;
        }
        n += (size_t)r;
        buf[n] = '\0';
        if (n >= 4 && strstr(buf, "\r\n\r\n")) {
            break;
        }
    }
    const char *body = "nope";
    if (strncmp(buf, "GET /a ", 7) == 0) {
        hits++;
        body = "page-a";
    } else if (strncmp(buf, "GET /b ", 7) == 0) {
        hits++;
        body = "page-b";
    } else if (strncmp(buf, "GET /count ", 11) == 0) {
        char tmp[64];
        snprintf(tmp, sizeof(tmp), "count=%d", hits);
        char msg[128];
        int m = snprintf(msg, sizeof(msg),
                         "HTTP/1.0 200 OK\r\nContent-Length: %zu\r\nConnection: close\r\n\r\n%s",
                         strlen(tmp), tmp);
        if (m > 0) {
            (void)write(fd, msg, (size_t)m);
        }
        return;
    }
    char msg[128];
    int m = snprintf(msg, sizeof(msg),
                     "HTTP/1.0 200 OK\r\nContent-Length: %zu\r\nConnection: close\r\n\r\n%s",
                     strlen(body), body);
    if (m > 0) {
        (void)write(fd, msg, (size_t)m);
    }
}

int main(void) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    int yes = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
    struct sockaddr_in a;
    memset(&a, 0, sizeof(a));
    a.sin_family = AF_INET;
    a.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    a.sin_port = htons(0);
    if (bind(fd, (struct sockaddr *)&a, sizeof(a)) < 0 || listen(fd, 16) < 0) {
        return 1;
    }
    socklen_t len = sizeof(a);
    getsockname(fd, (struct sockaddr *)&a, &len);
    printf("port %d\n", ntohs(a.sin_port));
    fflush(stdout);
    for (;;) {
        int c = accept(fd, NULL, NULL);
        if (c < 0) {
            continue;
        }
        serve(c);
        close(c);
    }
}
