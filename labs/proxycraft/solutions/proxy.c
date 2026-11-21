/* Concurrent HTTP/1.0 forward proxy with a small LRU cache.
 * Absolute-form request line: GET http://127.0.0.1:port/path HTTP/1.0
 * Only numeric IPv4 hosts are dialed. Each connection is one detached thread.
 */
#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define MAX_OBJ (32 * 1024)
#define CACHE_N 8

typedef struct {
    int used;
    unsigned long stamp;
    char url[384];
    unsigned char *data;
    size_t len;
} centry_t;

static centry_t cache[CACHE_N];
static unsigned long cache_clock;
static pthread_mutex_t cache_mu = PTHREAD_MUTEX_INITIALIZER;

static int listen_local(int *port) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        return -1;
    }
    int yes = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
    struct sockaddr_in a;
    memset(&a, 0, sizeof(a));
    a.sin_family = AF_INET;
    a.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    a.sin_port = htons(0);
    if (bind(fd, (struct sockaddr *)&a, sizeof(a)) < 0 || listen(fd, 32) < 0) {
        close(fd);
        return -1;
    }
    socklen_t len = sizeof(a);
    getsockname(fd, (struct sockaddr *)&a, &len);
    *port = ntohs(a.sin_port);
    return fd;
}

static int read_header(int fd, char *buf, size_t cap) {
    size_t n = 0;
    while (n + 1 < cap) {
        char c;
        ssize_t r = read(fd, &c, 1);
        if (r == 0) {
            break;
        }
        if (r < 0) {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }
        buf[n++] = c;
        buf[n] = '\0';
        if (n >= 4 && memcmp(buf + n - 4, "\r\n\r\n", 4) == 0) {
            return (int)n;
        }
    }
    return -1;
}

static int parse_request(const char *hdr, char *url, size_t url_n) {
    if (strncmp(hdr, "GET ", 4) != 0) {
        return -1;
    }
    const char *p = hdr + 4;
    const char *sp = strchr(p, ' ');
    if (!sp || (size_t)(sp - p) >= url_n) {
        return -1;
    }
    memcpy(url, p, (size_t)(sp - p));
    url[sp - p] = '\0';
    return strncmp(url, "http://", 7) == 0 ? 0 : -1;
}

static int parse_url(const char *url, char *host, size_t host_n, int *port, char *path,
                     size_t path_n) {
    const char *p = url + 7;
    const char *slash = strchr(p, '/');
    if (!slash) {
        return -1;
    }
    const char *colon = NULL;
    for (const char *q = p; q < slash; q++) {
        if (*q == ':') {
            colon = q;
        }
    }
    if (colon) {
        if ((size_t)(colon - p) >= host_n) {
            return -1;
        }
        memcpy(host, p, (size_t)(colon - p));
        host[colon - p] = '\0';
        *port = atoi(colon + 1);
    } else {
        if ((size_t)(slash - p) >= host_n) {
            return -1;
        }
        memcpy(host, p, (size_t)(slash - p));
        host[slash - p] = '\0';
        *port = 80;
    }
    if ((size_t)strlen(slash) >= path_n || *port <= 0) {
        return -1;
    }
    snprintf(path, path_n, "%s", slash);
    return 0;
}

static centry_t *cache_find(const char *url) {
    for (int i = 0; i < CACHE_N; i++) {
        if (cache[i].used && strcmp(cache[i].url, url) == 0) {
            cache[i].stamp = ++cache_clock;
            return &cache[i];
        }
    }
    return NULL;
}

static void cache_store(const char *url, const unsigned char *data, size_t len) {
    if (len == 0 || len > MAX_OBJ || strlen(url) >= sizeof(cache[0].url)) {
        return;
    }
    int slot = -1;
    unsigned long oldest = ~0ul;
    for (int i = 0; i < CACHE_N; i++) {
        if (!cache[i].used) {
            slot = i;
            break;
        }
        if (cache[i].stamp < oldest) {
            oldest = cache[i].stamp;
            slot = i;
        }
    }
    if (slot < 0) {
        return;
    }
    free(cache[slot].data);
    cache[slot].data = malloc(len);
    if (!cache[slot].data) {
        cache[slot].used = 0;
        return;
    }
    memcpy(cache[slot].data, data, len);
    cache[slot].len = len;
    snprintf(cache[slot].url, sizeof(cache[slot].url), "%s", url);
    cache[slot].stamp = ++cache_clock;
    cache[slot].used = 1;
}

static int dial(const char *host, int port) {
    struct in_addr addr;
    if (inet_pton(AF_INET, host, &addr) != 1) {
        return -1;
    }
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        return -1;
    }
    struct sockaddr_in a;
    memset(&a, 0, sizeof(a));
    a.sin_family = AF_INET;
    a.sin_addr = addr;
    a.sin_port = htons((uint16_t)port);
    if (connect(fd, (struct sockaddr *)&a, sizeof(a)) < 0) {
        close(fd);
        return -1;
    }
    return fd;
}

static void reply_error(int fd, const char *status) {
    char buf[128];
    int n = snprintf(buf, sizeof(buf), "HTTP/1.0 %s\r\nContent-Length: 0\r\nConnection: close\r\n\r\n",
                     status);
    if (n > 0) {
        (void)write(fd, buf, (size_t)n);
    }
}

static void *handle(void *arg) {
    int fd = *(int *)arg;
    free(arg);
    char hdr[2048];
    if (read_header(fd, hdr, sizeof(hdr)) < 0) {
        close(fd);
        return NULL;
    }
    char url[384];
    if (parse_request(hdr, url, sizeof(url)) != 0) {
        reply_error(fd, "400 Bad Request");
        close(fd);
        return NULL;
    }
    unsigned char *hit = NULL;
    size_t hit_len = 0;
    pthread_mutex_lock(&cache_mu);
    centry_t *e = cache_find(url);
    if (e) {
        hit = malloc(e->len);
        if (hit) {
            memcpy(hit, e->data, e->len);
            hit_len = e->len;
        }
    }
    pthread_mutex_unlock(&cache_mu);
    if (hit) {
        (void)write(fd, hit, hit_len);
        free(hit);
        close(fd);
        return NULL;
    }

    char host[64], path[256];
    int port = 0;
    if (parse_url(url, host, sizeof(host), &port, path, sizeof(path)) != 0) {
        reply_error(fd, "400 Bad Request");
        close(fd);
        return NULL;
    }
    int upstream = dial(host, port);
    if (upstream < 0) {
        reply_error(fd, "502 Bad Gateway");
        close(fd);
        return NULL;
    }
    char req[512];
    int rn = snprintf(req, sizeof(req),
                      "GET %s HTTP/1.0\r\nHost: %s:%d\r\nConnection: close\r\n\r\n", path, host,
                      port);
    if (rn < 0 || write(upstream, req, (size_t)rn) != rn) {
        close(upstream);
        reply_error(fd, "502 Bad Gateway");
        close(fd);
        return NULL;
    }
    unsigned char *body = malloc(MAX_OBJ);
    size_t n = 0;
    int overflow = 0;
    if (!body) {
        close(upstream);
        close(fd);
        return NULL;
    }
    while (n < MAX_OBJ) {
        ssize_t r = read(upstream, body + n, MAX_OBJ - n);
        if (r == 0) {
            break;
        }
        if (r < 0) {
            if (errno == EINTR) {
                continue;
            }
            break;
        }
        n += (size_t)r;
    }
    char extra;
    if (read(upstream, &extra, 1) == 1) {
        overflow = 1;
        (void)write(fd, body, n);
        (void)write(fd, &extra, 1);
        char buf[1024];
        ssize_t r;
        while ((r = read(upstream, buf, sizeof(buf))) > 0) {
            (void)write(fd, buf, (size_t)r);
        }
    } else if (n > 0 && memcmp(body, "HTTP/1.0 200", 12) == 0) {
        pthread_mutex_lock(&cache_mu);
        cache_store(url, body, n);
        pthread_mutex_unlock(&cache_mu);
        (void)write(fd, body, n);
    } else {
        (void)write(fd, body, n);
    }
    (void)overflow;
    free(body);
    close(upstream);
    close(fd);
    return NULL;
}

int main(void) {
    int port = 0;
    int fd = listen_local(&port);
    if (fd < 0) {
        perror("listen");
        return 1;
    }
    printf("port %d\n", port);
    fflush(stdout);
    for (;;) {
        int c = accept(fd, NULL, NULL);
        if (c < 0) {
            if (errno == EINTR) {
                continue;
            }
            break;
        }
        int *slot = malloc(sizeof(int));
        if (!slot) {
            close(c);
            continue;
        }
        *slot = c;
        pthread_t th;
        if (pthread_create(&th, NULL, handle, slot) != 0) {
            close(c);
            free(slot);
            continue;
        }
        pthread_detach(th);
    }
    return 0;
}
