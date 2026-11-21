/* Prethreaded echo. The main thread accepts; workers pop fds from a queue
 * guarded by a mutex and condition variables. Clients are threads in this
 * process, all on 127.0.0.1. There is no fork.
 */

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#define QCAP 2
#define NCLIENT 4
#define NWORKER 2

struct queue {
    int fds[QCAP];
    int head;
    int tail;
    int count;
    int shutdown;
    pthread_mutex_t mu;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;
};

static struct queue queue;

struct client_arg {
    int port;
    int id;
    int ok;
};

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
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGPIPE, &sa, NULL);
    alarm(5);
}

static void tune_timeout(int fd) {
    struct timeval tv;
    tv.tv_sec = 2;
    tv.tv_usec = 0;
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
}

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

static int read_line(int fd, char *buf, size_t cap) {
    size_t i = 0;
    if (cap == 0) {
        return -1;
    }
    while (i + 1 < cap) {
        char c;
        ssize_t r = read(fd, &c, 1);
        if (r < 0) {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }
        if (r == 0) {
            break;
        }
        buf[i++] = c;
        if (c == '\n') {
            break;
        }
    }
    buf[i] = '\0';
    return (int)i;
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

static int connect_loopback(int port) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    if (fd < 0) {
        return -1;
    }
    tune_timeout(fd);
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons((uint16_t)port);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        close(fd);
        return -1;
    }
    return fd;
}

static void queue_init(void) {
    memset(&queue, 0, sizeof(queue));
    pthread_mutex_init(&queue.mu, NULL);
    pthread_cond_init(&queue.not_empty, NULL);
    pthread_cond_init(&queue.not_full, NULL);
}

static int enqueue(int fd) {
    pthread_mutex_lock(&queue.mu);
    while (queue.count == QCAP && !queue.shutdown) {
        pthread_cond_wait(&queue.not_full, &queue.mu);
    }
    if (queue.shutdown) {
        pthread_mutex_unlock(&queue.mu);
        close(fd);
        return -1;
    }
    queue.fds[queue.tail] = fd;
    queue.tail = (queue.tail + 1) % QCAP;
    queue.count++;
    pthread_cond_signal(&queue.not_empty);
    pthread_mutex_unlock(&queue.mu);
    return 0;
}

static void *worker(void *arg) {
    (void)arg;
    for (;;) {
        int fd;
        char line[64];
        int n;
        pthread_mutex_lock(&queue.mu);
        while (queue.count == 0 && !queue.shutdown) {
            pthread_cond_wait(&queue.not_empty, &queue.mu);
        }
        if (queue.count == 0 && queue.shutdown) {
            pthread_mutex_unlock(&queue.mu);
            break;
        }
        fd = queue.fds[queue.head];
        queue.head = (queue.head + 1) % QCAP;
        queue.count--;
        pthread_cond_signal(&queue.not_full);
        pthread_mutex_unlock(&queue.mu);

        n = read_line(fd, line, sizeof(line));
        if (n > 0) {
            write_full(fd, line, (size_t)n);
        }
        close(fd);
    }
    return NULL;
}

static void *client_fn(void *arg) {
    struct client_arg *a = arg;
    int fd = connect_loopback(a->port);
    char msg[32];
    char line[64];
    int nmsg;
    int n;
    a->ok = 0;
    if (fd < 0) {
        return NULL;
    }
    nmsg = snprintf(msg, sizeof(msg), "m%d\n", a->id);
    if (nmsg < 0 || (size_t)nmsg >= sizeof(msg) || write_full(fd, msg, (size_t)nmsg) < 0) {
        close(fd);
        return NULL;
    }
    n = read_line(fd, line, sizeof(line));
    close(fd);
    if (n == nmsg && strcmp(line, msg) == 0) {
        a->ok = 1;
    }
    return NULL;
}

int main(void) {
    pthread_t workers[NWORKER];
    pthread_t clients[NCLIENT];
    struct client_arg args[NCLIENT];
    int port = 0;
    int lfd;
    int i;
    int ok = 1;

    arm_alarm();
    queue_init();
    lfd = listen_loopback(&port);
    if (lfd < 0) {
        perror("listen");
        return 1;
    }
    for (i = 0; i < NWORKER; i++) {
        if (pthread_create(&workers[i], NULL, worker, NULL) != 0) {
            return 1;
        }
    }
    for (i = 0; i < NCLIENT; i++) {
        args[i].port = port;
        args[i].id = i;
        args[i].ok = 0;
        if (pthread_create(&clients[i], NULL, client_fn, &args[i]) != 0) {
            return 1;
        }
    }
    for (i = 0; i < NCLIENT; i++) {
        struct sockaddr_in cli;
        socklen_t len = sizeof(cli);
        int c = accept(lfd, (struct sockaddr *)&cli, &len);
        if (c < 0) {
            if (errno == EINTR) {
                i--;
                continue;
            }
            ok = 0;
            break;
        }
        tune_timeout(c);
        if (enqueue(c) < 0) {
            ok = 0;
            break;
        }
    }
    for (i = 0; i < NCLIENT; i++) {
        pthread_join(clients[i], NULL);
        if (!args[i].ok) {
            ok = 0;
        }
    }
    pthread_mutex_lock(&queue.mu);
    queue.shutdown = 1;
    pthread_cond_broadcast(&queue.not_empty);
    pthread_cond_broadcast(&queue.not_full);
    pthread_mutex_unlock(&queue.mu);
    for (i = 0; i < NWORKER; i++) {
        pthread_join(workers[i], NULL);
    }
    close(lfd);
    pthread_mutex_destroy(&queue.mu);
    pthread_cond_destroy(&queue.not_empty);
    pthread_cond_destroy(&queue.not_full);
    if (!ok) {
        fprintf(stderr, "prethread exchange failed\n");
        return 1;
    }
    printf("prethread-ok\n");
    return 0;
}
