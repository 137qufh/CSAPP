/* HTTP/1.0 client built on sockets. No libcurl. */

#include "http_client.h"

#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

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

static const char *find_header_end(const char *buf, size_t n) {
    size_t i;
    if (n < 4) {
        return NULL;
    }
    for (i = 0; i + 3 < n; i++) {
        if (buf[i] == '\r' && buf[i + 1] == '\n' && buf[i + 2] == '\r' &&
            buf[i + 3] == '\n') {
            return buf + i;
        }
    }
    return NULL;
}

static int header_content_length(const char *hdr, size_t hdr_len, size_t *out) {
    size_t i = 0;
    while (i < hdr_len) {
        size_t line = i;
        while (i + 1 < hdr_len && !(hdr[i] == '\r' && hdr[i + 1] == '\n')) {
            i++;
        }
        if (i + 1 >= hdr_len) {
            break;
        }
        if ((size_t)(i - line) >= 15 && strncasecmp(hdr + line, "Content-Length:", 15) == 0) {
            const char *p = hdr + line + 15;
            while (p < hdr + i && (*p == ' ' || *p == '\t')) {
                p++;
            }
            if (p < hdr + i && isdigit((unsigned char)*p)) {
                *out = (size_t)strtoul(p, NULL, 10);
                return 1;
            }
            return 0;
        }
        i += 2;
    }
    return 0;
}

int http_fetch(const char *host, int port, const char *path, int *status_out,
               unsigned char **body_out, size_t *body_len) {
    int fd;
    struct sockaddr_in addr;
    char req[1024];
    char hdr[8192];
    size_t hlen = 0;
    const char *end;
    size_t header_bytes;
    int status = 0;
    size_t content_length = 0;
    int have_len;
    size_t already;
    unsigned char *body;
    size_t blen;
    int n;

    if (body_out) {
        *body_out = NULL;
    }
    if (body_len) {
        *body_len = 0;
    }
    if (!host || !path || port <= 0 || port > 65535) {
        return -1;
    }
    {
        struct sigaction sa;
        memset(&sa, 0, sizeof(sa));
        sa.sa_handler = SIG_IGN;
        sigemptyset(&sa.sa_mask);
        sigaction(SIGPIPE, &sa, NULL);
    }

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        return -1;
    }
    tune_timeout(fd);
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons((uint16_t)port);
    if (inet_pton(AF_INET, host, &addr.sin_addr) != 1) {
        close(fd);
        return -1;
    }
    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        close(fd);
        return -1;
    }

    n = snprintf(req, sizeof(req),
                 "GET %s HTTP/1.0\r\n"
                 "Host: %s:%d\r\n"
                 "Connection: close\r\n"
                 "\r\n",
                 path, host, port);
    if (n < 0 || (size_t)n >= sizeof(req) || write_full(fd, req, (size_t)n) < 0) {
        close(fd);
        return -1;
    }

    end = NULL;
    while (hlen + 1 < sizeof(hdr)) {
        ssize_t r = read(fd, hdr + hlen, sizeof(hdr) - 1 - hlen);
        if (r < 0) {
            if (errno == EINTR) {
                continue;
            }
            close(fd);
            return -1;
        }
        if (r == 0) {
            break;
        }
        hlen += (size_t)r;
        hdr[hlen] = '\0';
        end = find_header_end(hdr, hlen);
        if (end) {
            break;
        }
    }
    if (!end) {
        close(fd);
        return -1;
    }
    header_bytes = (size_t)(end - hdr) + 4;
    if (sscanf(hdr, "HTTP/%*s %d", &status) != 1) {
        close(fd);
        return -1;
    }
    have_len = header_content_length(hdr, header_bytes, &content_length);
    already = hlen - header_bytes;

    if (have_len) {
        if (already > content_length) {
            already = content_length;
        }
        body = malloc(content_length + 1);
        if (!body) {
            close(fd);
            return -1;
        }
        memcpy(body, hdr + header_bytes, already);
        blen = already;
        while (blen < content_length) {
            ssize_t r = read(fd, body + blen, content_length - blen);
            if (r < 0) {
                if (errno == EINTR) {
                    continue;
                }
                free(body);
                close(fd);
                return -1;
            }
            if (r == 0) {
                break;
            }
            blen += (size_t)r;
        }
        body[blen] = '\0';
    } else {
        size_t cap = already + 4096;
        if (cap < 4096) {
            cap = 4096;
        }
        body = malloc(cap);
        if (!body) {
            close(fd);
            return -1;
        }
        memcpy(body, hdr + header_bytes, already);
        blen = already;
        for (;;) {
            ssize_t r;
            if (blen + 1 >= cap) {
                size_t ncap = cap * 2;
                unsigned char *grown = realloc(body, ncap);
                if (!grown) {
                    free(body);
                    close(fd);
                    return -1;
                }
                body = grown;
                cap = ncap;
            }
            r = read(fd, body + blen, cap - 1 - blen);
            if (r < 0) {
                if (errno == EINTR) {
                    continue;
                }
                free(body);
                close(fd);
                return -1;
            }
            if (r == 0) {
                break;
            }
            blen += (size_t)r;
        }
        body[blen] = '\0';
    }

    close(fd);
    if (status_out) {
        *status_out = status;
    }
    if (body_len) {
        *body_len = blen;
    }
    if (body_out) {
        *body_out = body;
    } else {
        free(body);
    }
    return 0;
}
