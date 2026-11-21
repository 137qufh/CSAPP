#include "ex_counter.h"
#include "minitest.h"

#include <pthread.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

struct add_job {
    safe_counter_t *counter;
    int times;
};

static void on_alarm(int sig) {
    (void)sig;
    _exit(2);
}

static void *worker(void *arg) {
    struct add_job *job = arg;
    int i;
    for (i = 0; i < job->times; i++) {
        sc_add(job->counter, 1);
    }
    return NULL;
}

int main(void) {
    safe_counter_t counter;
    struct add_job job;
    pthread_t threads[4];
    struct sigaction sa;
    int i;

    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = on_alarm;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGALRM, &sa, NULL);
    alarm(5);

    sc_init(&counter);
    job.counter = &counter;
    job.times = 1000;
    for (i = 0; i < 4; i++) {
        MT_ASSERT(pthread_create(&threads[i], NULL, worker, &job) == 0);
    }
    for (i = 0; i < 4; i++) {
        pthread_join(threads[i], NULL);
    }
    MT_EQ_INT(sc_get(&counter), 4000);
    sc_destroy(&counter);
    MT_REPORT();
}
