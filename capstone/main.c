#include "link.h"
#include "memlib.h"
#include "memsys.h"
#include "mm.h"
#include "my86.h"

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

static my86_t *cpu;
static memsys_t *ms;

static int listen_local(int *port) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    int yes = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
    struct sockaddr_in a;
    memset(&a, 0, sizeof(a));
    a.sin_family = AF_INET;
    a.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    a.sin_port = htons(0);
    if (bind(fd, (struct sockaddr *)&a, sizeof(a)) < 0 || listen(fd, 16) < 0) {
        return -1;
    }
    socklen_t len = sizeof(a);
    getsockname(fd, (struct sockaddr *)&a, &len);
    *port = ntohs(a.sin_port);
    return fd;
}

static void json_stats(char *buf, size_t n) {
    unsigned long hits = 0, misses = 0, evict = 0;
    uint64_t th = 0, tm = 0;
    memsys_stats(ms, &hits, &misses, &evict, &th, &tm);
    double cpi = cpu->steps ? (double)cpu->steps / (double)cpu->steps : 0;
    double ch = (hits + misses) ? (double)hits / (double)(hits + misses) : 0;
    double tr = (th + tm) ? (double)th / (double)(th + tm) : 0;
    size_t heap = mem_heap_size();
    snprintf(buf, n,
             "{\"steps\":%llu,\"cycles\":%llu,\"cpi\":%.2f,\"cache_hits\":%lu,"
             "\"cache_misses\":%lu,\"cache_evictions\":%lu,\"cache_hit_rate\":%.3f,"
             "\"tlb_hits\":%llu,\"tlb_misses\":%llu,\"tlb_hit_rate\":%.3f,"
             "\"heap_bytes\":%zu,\"halted\":%d}\n",
             (unsigned long long)cpu->steps, (unsigned long long)cpu->steps, cpi, hits, misses,
             evict, ch, (unsigned long long)th, (unsigned long long)tm, tr, heap, cpu->halted);
    (void)cpi;
}

static void send_all(int fd, const char *s) {
    size_t n = strlen(s);
    size_t off = 0;
    while (off < n) {
        ssize_t w = write(fd, s + off, n - off);
        if (w < 0) {
            if (errno == EINTR) {
                continue;
            }
            return;
        }
        off += (size_t)w;
    }
}

static void *http_main(void *arg) {
    int fd = *(int *)arg;
    free(arg);
    for (;;) {
        int c = accept(fd, NULL, NULL);
        if (c < 0) {
            continue;
        }
        char req[512];
        size_t n = 0;
        while (n + 1 < sizeof(req)) {
            ssize_t r = read(c, req + n, 1);
            if (r <= 0) {
                break;
            }
            n += (size_t)r;
            req[n] = '\0';
            if (n >= 4 && strstr(req, "\r\n\r\n")) {
                break;
            }
        }
        char stats[512];
        json_stats(stats, sizeof(stats));
        int want_json = strncmp(req, "GET /stats", 10) == 0;
        char hdr[256];
        if (want_json) {
            snprintf(hdr, sizeof(hdr),
                     "HTTP/1.0 200 OK\r\nContent-Type: application/json\r\nContent-Length: %zu\r\n"
                     "Connection: close\r\n\r\n",
                     strlen(stats));
            send_all(c, hdr);
            send_all(c, stats);
        } else {
            char page[1600];
            snprintf(page, sizeof(page),
                     "<!doctype html><meta charset=utf-8><title>syscraft</title>"
                     "<h1>syscraft</h1><p>顺序 MY86 核 + TLB/页表 + cache 统计 + 分配器。</p>"
                     "<pre>%s</pre><p><a href=\"/stats\">/stats</a></p>",
                     stats);
            snprintf(hdr, sizeof(hdr),
                     "HTTP/1.0 200 OK\r\nContent-Type: text/html\r\nContent-Length: %zu\r\n"
                     "Connection: close\r\n\r\n",
                     strlen(page));
            send_all(c, hdr);
            send_all(c, page);
        }
        close(c);
    }
    return NULL;
}

static int guest_svc(my86_t *m, int num) {
    if (num == 0) {
        return 1;
    }
    if (num == 1) {
        if (m->out_len + 1 < (int)sizeof(m->out)) {
            m->out[m->out_len++] = (char)(m->reg[1] & 0xffu);
            m->out[m->out_len] = '\0';
        }
        return 0;
    }
    if (num == 2) {
        char tmp[32];
        int n = snprintf(tmp, sizeof(tmp), "%d\n", (int)m->reg[1]);
        for (int i = 0; i < n && m->out_len + 1 < (int)sizeof(m->out); i++) {
            m->out[m->out_len++] = tmp[i];
        }
        m->out[m->out_len] = '\0';
        return 0;
    }
    if (num == 3) {
        size_t sz = m->reg[1] ? m->reg[1] : 1;
        void *p = mm_malloc(sz);
        if (!p) {
            m->reg[1] = 0;
            return 0;
        }
        unsigned char *base = mem_heap_lo();
        m->reg[1] = 0x8000u + (uint32_t)((unsigned char *)p - base);
        return 0;
    }
    if (num == 4) {
        uint32_t va = m->reg[1];
        if (va >= 0x8000u) {
            unsigned char *p = (unsigned char *)mem_heap_lo() + (va - 0x8000u);
            mm_free(p);
        }
        return 0;
    }
    return -1;
}

static char *read_file(const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) {
        return NULL;
    }
    fseek(f, 0, SEEK_END);
    long n = ftell(f);
    rewind(f);
    char *b = malloc((size_t)n + 1);
    if (!b || fread(b, 1, (size_t)n, f) != (size_t)n) {
        free(b);
        fclose(f);
        return NULL;
    }
    b[n] = '\0';
    fclose(f);
    return b;
}

static int run_sources(char **srcs, int n, char *err, int errlen) {
    char *linked = link_modules(srcs, n, err, errlen);
    if (!linked) {
        return -1;
    }
    my86_t *tmp = my86_new();
    if (my86_assemble(tmp, linked) != 0) {
        snprintf(err, (size_t)errlen, "%s", tmp->err[0] ? tmp->err : "assemble failed");
        my86_dispose(tmp);
        free(linked);
        return -1;
    }
    my86_reset(cpu);
    memcpy(cpu->mem, tmp->mem, 0x8000);
    cpu->pc = 0;
    int rc = my86_run_seq(cpu, 1000000);
    if (rc != 0) {
        snprintf(err, (size_t)errlen, "%s", cpu->err);
    }
    my86_dispose(tmp);
    free(linked);
    return rc;
}

static void print_regs(void) {
    for (int i = 0; i < 8; i++) {
        uint32_t u = cpu->reg[i];
        float f;
        memcpy(&f, &u, sizeof(f));
        printf("r%d  hex=%08x  unsigned=%u  signed=%d  floatbits=%.6g\n", i, u, u, (int)u, f);
    }
}

static int self_get(int port, const char *path, char *out, size_t n) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in a;
    memset(&a, 0, sizeof(a));
    a.sin_family = AF_INET;
    a.sin_port = htons((uint16_t)port);
    inet_pton(AF_INET, "127.0.0.1", &a.sin_addr);
    if (connect(fd, (struct sockaddr *)&a, sizeof(a)) < 0) {
        close(fd);
        return -1;
    }
    char req[128];
    snprintf(req, sizeof(req), "GET %s HTTP/1.0\r\nHost: localhost\r\n\r\n", path);
    send_all(fd, req);
    size_t got = 0;
    while (got + 1 < n) {
        ssize_t r = read(fd, out + got, n - 1 - got);
        if (r <= 0) {
            break;
        }
        got += (size_t)r;
    }
    out[got] = '\0';
    close(fd);
    return (int)got;
}

static int demo(int http_port) {
    char err[160];
    char *sum = read_file("programs/sum.my");
    char *ma = read_file("programs/mod_main.my");
    char *mb = read_file("programs/mod_add.my");
    char *heap = read_file("programs/heap.my");
    if (!sum || !ma || !mb || !heap) {
        fprintf(stderr, "missing programs/ (run from capstone/)\n");
        return 1;
    }
    char *one[] = {sum};
    if (run_sources(one, 1, err, sizeof(err)) != 0) {
        fprintf(stderr, "sum: %s\n", err);
        return 1;
    }
    printf("sum out: %s", cpu->out);
    if (!strstr(cpu->out, "55\n") || cpu->reg[1] != 55) {
        fprintf(stderr, "sum mismatch\n");
        return 1;
    }
    char *mods[] = {ma, mb};
    if (run_sources(mods, 2, err, sizeof(err)) != 0) {
        fprintf(stderr, "link: %s\n", err);
        return 1;
    }
    printf("link out: %s", cpu->out);
    if (!strstr(cpu->out, "22\n") || cpu->reg[1] != 22) {
        fprintf(stderr, "link mismatch r1=%u out=%s\n", cpu->reg[1], cpu->out);
        return 1;
    }
    char *hs[] = {heap};
    if (run_sources(hs, 1, err, sizeof(err)) != 0) {
        fprintf(stderr, "heap: %s\n", err);
        return 1;
    }
    printf("heap out: %s", cpu->out);
    if (!strstr(cpu->out, "65\n")) {
        fprintf(stderr, "heap mismatch\n");
        return 1;
    }
    char page[2048];
    if (self_get(http_port, "/stats", page, sizeof(page)) < 0 || !strstr(page, "cache_misses")) {
        fprintf(stderr, "stats http failed\n%s\n", page);
        return 1;
    }
    if (self_get(http_port, "/", page, sizeof(page)) < 0 || !strstr(page, "syscraft")) {
        fprintf(stderr, "dashboard failed\n");
        return 1;
    }
    char stats[512];
    json_stats(stats, sizeof(stats));
    printf("stats %s", stats);
    printf("demo-ok\n");
    free(sum);
    free(ma);
    free(mb);
    free(heap);
    return 0;
}

static void shell_loop(void) {
    char line[256];
    printf("syscraft> ");
    fflush(stdout);
    while (fgets(line, sizeof(line), stdin)) {
        if (strncmp(line, "quit", 4) == 0) {
            break;
        }
        if (strncmp(line, "help", 4) == 0) {
            printf("run <file.my> | regs | stats | quit\n");
        } else if (strncmp(line, "regs", 4) == 0) {
            print_regs();
        } else if (strncmp(line, "stats", 5) == 0) {
            char stats[512];
            json_stats(stats, sizeof(stats));
            fputs(stats, stdout);
        } else if (strncmp(line, "run ", 4) == 0) {
            char *nl = strchr(line, '\n');
            if (nl) {
                *nl = '\0';
            }
            char *src = read_file(line + 4);
            if (!src) {
                printf("cannot read %s\n", line + 4);
            } else {
                char err[160];
                char *one[] = {src};
                if (run_sources(one, 1, err, sizeof(err)) != 0) {
                    printf("error: %s\n", err);
                } else {
                    fputs(cpu->out, stdout);
                    print_regs();
                }
                free(src);
            }
        } else if (line[0] != '\n') {
            printf("unknown command\n");
        }
        printf("syscraft> ");
        fflush(stdout);
    }
}

int main(int argc, char **argv) {
    int demo_mode = argc > 1 && strcmp(argv[1], "--demo") == 0;
    cpu = my86_new();
    ms = memsys_new(cpu);
    cpu->svc = guest_svc;
    mem_init_buf(cpu->mem + 0x8000, 0x4000);
    if (mm_init() != 0) {
        fprintf(stderr, "mm_init failed\n");
        return 1;
    }
    int port = 0;
    int fd = listen_local(&port);
    if (fd < 0) {
        perror("listen");
        return 1;
    }
    int *arg = malloc(sizeof(int));
    *arg = fd;
    pthread_t th;
    pthread_create(&th, NULL, http_main, arg);
    pthread_detach(th);
    printf("status-port %d\n", port);
    fflush(stdout);
    int rc = 0;
    if (demo_mode) {
        rc = demo(port);
    } else {
        shell_loop();
    }
    return rc;
}
