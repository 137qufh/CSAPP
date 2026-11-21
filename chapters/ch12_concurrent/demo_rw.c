/* Readers share a pthread_rwlock; writers take it exclusively.
 * After every thread joins, the counter must sit at a stable final value.
 */

#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define NREAD 4
#define NWRITE 2
#define NSTEPS 100

static int shared = 0;
static pthread_rwlock_t rw = PTHREAD_RWLOCK_INITIALIZER;

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

static void *writer(void *arg) {
    int i;
    (void)arg;
    for (i = 0; i < NSTEPS; i++) {
        pthread_rwlock_wrlock(&rw);
        shared++;
        pthread_rwlock_unlock(&rw);
    }
    return NULL;
}

static void *reader(void *arg) {
    int *last = arg;
    int seen = 0;
    int i;
    for (i = 0; i < 40; i++) {
        pthread_rwlock_rdlock(&rw);
        seen = shared;
        pthread_rwlock_unlock(&rw);
    }
    *last = seen;
    return NULL;
}

int main(void) {
    pthread_t readers[NREAD];
    pthread_t writers[NWRITE];
    int last[NREAD];
    int final_value;
    int i;

    arm_alarm();
    for (i = 0; i < NREAD; i++) {
        last[i] = -1;
        if (pthread_create(&readers[i], NULL, reader, &last[i]) != 0) {
            perror("pthread_create");
            return 1;
        }
    }
    for (i = 0; i < NWRITE; i++) {
        if (pthread_create(&writers[i], NULL, writer, NULL) != 0) {
            perror("pthread_create");
            return 1;
        }
    }
    for (i = 0; i < NREAD; i++) {
        pthread_join(readers[i], NULL);
    }
    for (i = 0; i < NWRITE; i++) {
        pthread_join(writers[i], NULL);
    }
    pthread_rwlock_rdlock(&rw);
    final_value = shared;
    pthread_rwlock_unlock(&rw);
    pthread_rwlock_destroy(&rw);

    printf("value=%d\n", final_value);
    if (final_value != NWRITE * NSTEPS) {
        fprintf(stderr, "writers lost updates\n");
        return 1;
    }
    for (i = 0; i < NREAD; i++) {
        if (last[i] < 0 || last[i] > final_value) {
            fprintf(stderr, "reader saw %d\n", last[i]);
            return 1;
        }
    }
    printf("rw-ok\n");
    return 0;
}
