/* Four threads sum disjoint slices. Each thread writes only its own result. */

#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define N 1000
#define T 4

static int values[N];

struct slice {
    int begin;
    int end;
    long long sum;
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
    alarm(5);
}

static void *add_slice(void *arg) {
    struct slice *s = arg;
    long long sum = 0;
    int i;
    for (i = s->begin; i < s->end; i++) {
        sum += values[i];
    }
    s->sum = sum;
    return NULL;
}

int main(void) {
    struct slice slices[T];
    pthread_t threads[T];
    long long total = 0;
    long long expect = (long long)N * (N + 1) / 2;
    int i;
    int step = N / T;

    arm_alarm();
    for (i = 0; i < N; i++) {
        values[i] = i + 1;
    }
    for (i = 0; i < T; i++) {
        slices[i].begin = i * step;
        slices[i].end = (i == T - 1) ? N : (i + 1) * step;
        slices[i].sum = 0;
        if (pthread_create(&threads[i], NULL, add_slice, &slices[i]) != 0) {
            perror("pthread_create");
            return 1;
        }
    }
    for (i = 0; i < T; i++) {
        pthread_join(threads[i], NULL);
        total += slices[i].sum;
    }
    printf("total=%lld\n", total);
    if (total != expect) {
        fprintf(stderr, "sum mismatch\n");
        return 1;
    }
    printf("threads-ok\n");
    return 0;
}
