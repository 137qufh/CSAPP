/* A shared counter protected by a mutex.
 *
 * Without the lock, counter++ is a load, an add, and a store. Two threads
 * can load the same value and both store value+1, so one increment disappears.
 * That racy counter is not run here: its final value depends on the schedule,
 * so a test must not assert on it. The locked counter below is exact.
 */

#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define NTHREADS 4
#define NINC 10000

static int counter = 0;
static pthread_mutex_t mu = PTHREAD_MUTEX_INITIALIZER;

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

static void *worker(void *arg) {
    int i;
    (void)arg;
    for (i = 0; i < NINC; i++) {
        pthread_mutex_lock(&mu);
        counter++;
        pthread_mutex_unlock(&mu);
    }
    return NULL;
}

int main(void) {
    pthread_t threads[NTHREADS];
    int i;

    arm_alarm();
    for (i = 0; i < NTHREADS; i++) {
        if (pthread_create(&threads[i], NULL, worker, NULL) != 0) {
            perror("pthread_create");
            return 1;
        }
    }
    for (i = 0; i < NTHREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    printf("counter=%d\n", counter);
    pthread_mutex_destroy(&mu);
    if (counter != NTHREADS * NINC) {
        fprintf(stderr, "locked counter is not exact\n");
        return 1;
    }
    printf("racefix-ok\n");
    return 0;
}
