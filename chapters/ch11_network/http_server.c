/* Small original HTTP/1.0 server.
 * Static files come from ./www. A path under /cgi/ runs the matching
 * program in ./www/cgi-bin and, when that program prints only a body,
 * this server adds the status line and Content-Length.
 */

#include "http_server.h"

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>

#define REQ_CAP 8192
#define CGI_CAP 65536

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

static void tune_timeout(int fd) {
    struct timeval tv;
    tv.tv_sec = 2;
    tv.tv_usec = 0;
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
}

static int send_headers(int fd, int status, const char *reason, const char *type,
                        size_t len) {
    char hdr[512];
    int n = snprintf(hdr, sizeof(hdr),
                     "HTTP/1.0 %d %s\r\n"
                     "Content-Type: %s\r\n"
                     "Content-Length: %zu\r\n"
                     "Connection: close\r\n"
                     "\r\n",
                     status, reason, type, len);
    if (n < 0 || (size_t)n >= sizeof(hdr)) {
        return -1;
    }
    return write_full(fd, hdr, (size_t)n);
}

static int send_text(int fd, int status, const char *reason, const char *body) {
    size_t len = strlen(body);
    if (send_headers(fd, status, reason, "text/plain; charset=utf-8", len) < 0) {
        return -1;
    }
    return write_full(fd, body, len);
}

static int read_request(int fd, char *buf, size_t cap) {
    size_t n = 0;
    if (cap == 0) {
        return -1;
    }
    while (n + 1 < cap) {
        ssize_t r = read(fd, buf + n, cap - 1 - n);
        char *end;
        if (r < 0) {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }
        if (r == 0) {
            break;
        }
        n += (size_t)r;
        buf[n] = '\0';
        end = strstr(buf, "\r\n\r\n");
        if (end) {
            return 0;
        }
    }
    return -1;
}

static int join_path(char *dst, size_t dst_sz, const char *a, const char *b) {
    size_t na = strlen(a);
    size_t nb = strlen(b);
    if (na + nb + 1 > dst_sz) {
        return -1;
    }
    memcpy(dst, a, na);
    memcpy(dst + na, b, nb + 1);
    return 0;
}

static int reply_file(int fd, const char *full) {
    int file;
    struct stat st;
    const char *type = "application/octet-stream";
    size_t left;
    file = open(full, O_RDONLY);
    if (file < 0) {
        send_text(fd, 404, "Not Found", "not found\n");
        return -1;
    }
    if (fstat(file, &st) < 0 || !S_ISREG(st.st_mode) || st.st_size < 0) {
        close(file);
        send_text(fd, 404, "Not Found", "not found\n");
        return -1;
    }
    if (strstr(full, ".html")) {
        type = "text/html; charset=utf-8";
    } else if (strstr(full, ".txt")) {
        type = "text/plain; charset=utf-8";
    }
    left = (size_t)st.st_size;
    if (send_headers(fd, 200, "OK", type, left) < 0) {
        close(file);
        return -1;
    }
    while (left > 0) {
        char chunk[4096];
        size_t want = left < sizeof(chunk) ? left : sizeof(chunk);
        ssize_t r = read(file, chunk, want);
        if (r < 0) {
            if (errno == EINTR) {
                continue;
            }
            close(file);
            return -1;
        }
        if (r == 0) {
            close(file);
            return -1;
        }
        if (write_full(fd, chunk, (size_t)r) < 0) {
            close(file);
            return -1;
        }
        left -= (size_t)r;
    }
    close(file);
    return 0;
}

static int script_name_ok(const char *name) {
    const unsigned char *p;
    if (!name || name[0] == '\0') {
        return 0;
    }
    for (p = (const unsigned char *)name; *p; p++) {
        if (!isalnum(*p) && *p != '_' && *p != '-') {
            return 0;
        }
    }
    return 1;
}

static int reply_program(int client, const char *bin, const char *query) {
    int sp[2];
    pid_t pid;
    char *body;
    size_t len = 0;
    int status = 0;
    char qbuf[1024];
    const char *prefix = "QUERY_STRING=";
    size_t prefix_len;
    size_t qlen;

    if (access(bin, X_OK) != 0) {
        send_text(client, 404, "Not Found", "not found\n");
        return -1;
    }
    prefix_len = strlen(prefix);
    qlen = strlen(query);
    if (prefix_len + qlen + 1 > sizeof(qbuf)) {
        send_text(client, 400, "Bad Request", "query too long\n");
        return -1;
    }
    memcpy(qbuf, prefix, prefix_len);
    memcpy(qbuf + prefix_len, query, qlen + 1);

    if (pipe(sp) < 0) {
        send_text(client, 500, "Error", "pipe failed\n");
        return -1;
    }
    pid = fork();
    if (pid < 0) {
        close(sp[0]);
        close(sp[1]);
        send_text(client, 500, "Error", "fork failed\n");
        return -1;
    }
    if (pid == 0) {
        int devnull = open("/dev/null", O_RDONLY);
        char *argv[2];
        char *envp[3];
        if (devnull >= 0) {
            dup2(devnull, STDIN_FILENO);
            close(devnull);
        }
        if (dup2(sp[1], STDOUT_FILENO) < 0) {
            _exit(127);
        }
        close(sp[0]);
        close(sp[1]);
        close(client);
        argv[0] = (char *)bin;
        argv[1] = NULL;
        envp[0] = qbuf;
        envp[1] = "PATH=/usr/bin:/bin";
        envp[2] = NULL;
        execve(bin, argv, envp);
        _exit(127);
    }

    close(sp[1]);
    body = malloc(CGI_CAP);
    if (!body) {
        close(sp[0]);
        kill(pid, SIGKILL);
        waitpid(pid, &status, 0);
        send_text(client, 500, "Error", "out of memory\n");
        return -1;
    }
    while (len < CGI_CAP) {
        ssize_t r = read(sp[0], body + len, CGI_CAP - len);
        if (r < 0) {
            if (errno == EINTR) {
                continue;
            }
            break;
        }
        if (r == 0) {
            break;
        }
        len += (size_t)r;
    }
    close(sp[0]);
    if (waitpid(pid, &status, 0) < 0) {
        free(body);
        return -1;
    }
    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        send_text(client, 500, "Error", "program failed\n");
        free(body);
        return -1;
    }
    if (len >= 5 && memcmp(body, "HTTP/", 5) == 0) {
        if (write_full(client, body, len) < 0) {
            free(body);
            return -1;
        }
    } else if (send_headers(client, 200, "OK", "text/plain; charset=utf-8", len) < 0 ||
               write_full(client, body, len) < 0) {
        free(body);
        return -1;
    }
    free(body);
    return 0;
}

static void handle_client(int fd) {
    char req[REQ_CAP];
    char line[1024];
    char method[16];
    char uri[768];
    char version[32];
    char path[768];
    char query[768];
    char *mark;
    size_t i;
    size_t ncopy;

    if (read_request(fd, req, sizeof(req)) < 0) {
        send_text(fd, 400, "Bad Request", "bad request\n");
        return;
    }
    i = 0;
    while (req[i] != '\0' && req[i] != '\n' && i + 1 < sizeof(line)) {
        line[i] = req[i];
        i++;
    }
    line[i] = '\0';
    if (i > 0 && line[i - 1] == '\r') {
        line[i - 1] = '\0';
    }
    if (sscanf(line, "%15s %767s %31s", method, uri, version) != 3) {
        send_text(fd, 400, "Bad Request", "bad request\n");
        return;
    }
    if (strcmp(method, "GET") != 0) {
        send_text(fd, 405, "Method Not Allowed", "only GET\n");
        return;
    }
    if (strncmp(version, "HTTP/", 5) != 0) {
        send_text(fd, 400, "Bad Request", "bad request\n");
        return;
    }

    ncopy = strlen(uri);
    if (ncopy + 1 > sizeof(path)) {
        send_text(fd, 400, "Bad Request", "path too long\n");
        return;
    }
    memcpy(path, uri, ncopy + 1);
    query[0] = '\0';
    mark = strchr(path, '?');
    if (mark) {
        size_t qn = strlen(mark + 1);
        if (qn + 1 > sizeof(query)) {
            send_text(fd, 400, "Bad Request", "query too long\n");
            return;
        }
        memcpy(query, mark + 1, qn + 1);
        *mark = '\0';
    }
    if (path[0] != '/' || strstr(path, "..") || strstr(path, "//")) {
        send_text(fd, 400, "Bad Request", "bad path\n");
        return;
    }

    if (strncmp(path, "/cgi/", 5) == 0) {
        char bin[512];
        const char *script = path + 5;
        if (!script_name_ok(script) || join_path(bin, sizeof(bin), "www/cgi-bin/", script) < 0) {
            send_text(fd, 404, "Not Found", "not found\n");
            return;
        }
        reply_program(fd, bin, query);
        return;
    }

    {
        char full[1024];
        const char *rel = path;
        if (strcmp(path, "/") == 0) {
            rel = "/index.html";
        }
        if (join_path(full, sizeof(full), "www", rel) < 0) {
            send_text(fd, 400, "Bad Request", "path too long\n");
            return;
        }
        reply_file(fd, full);
    }
}

int http_serve_fd(int listen_fd) {
    int flags;
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGPIPE, &sa, NULL);

    flags = fcntl(listen_fd, F_GETFD);
    if (flags >= 0) {
        fcntl(listen_fd, F_SETFD, flags | FD_CLOEXEC);
    }

    for (;;) {
        struct sockaddr_in cli;
        socklen_t len = sizeof(cli);
        int client = accept(listen_fd, (struct sockaddr *)&cli, &len);
        if (client < 0) {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }
        tune_timeout(client);
        handle_client(client);
        close(client);
    }
}
