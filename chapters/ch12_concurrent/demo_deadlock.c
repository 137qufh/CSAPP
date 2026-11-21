/* Two mutexes. lock_ordered always locks the lower address first, so two
 * threads can call it together without waiting on each other in a cycle.
 *
 * The unsafe order is not compiled. It would look like this:
 *
 *   thread A: lock mu_a, then lock mu_b
 *   thread B: lock mu_b, then lock mu_a
 *
 * Each thread can hold one mutex and block forever on the other.
 */

#include <pthread.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static pthread_mutex_t mu_a = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t mu_b = PTHREAD_MUTEX_INITIALIZER;

#if 0
void lock_bad_demo(void) {
    pthread_mutex_lock(&mu_a);
    pthread_mutex_lock(&mu_b);
    pthread_mutex_unlock(&mu_b);
    pthread_mutex_unlock(&mu_a);
}
#endif

void lock_ordered(void) {
    pthread_mutex_t *first = &mu_a;
    pthread_mutex_t *second = &mu_b;
    if ((uintptr_t)second < (uintptr_t)first) {
        pthread_mutex_t *tmp = first;
        first = second;
        second = tmp;
    }
    pthread_mutex_lock(first);
    pthread_mutex_lock(second);
    pthread_mutex_unlock(second);
    pthread_mutex_unlock(first);
}

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

struct start_gate {
    pthread_mutex_t mu;
    pthread_cond_t cv;
    int ready;
    int go;
};

static struct start_gate gate;

static void *worker(void *arg) {
    int i;
    (void)arg;
    pthread_mutex_lock(&gate.mu);
    gate.ready++;
    if (gate.ready == 2) {
        pthread_cond_broadcast(&gate.cv);
    }
    while (!gate.go) {
        pthread_cond_wait(&gate.cv, &gate.mu);
    }
    pthread_mutex_unlock(&gate.mu);
    for (i = 0; i < 3000; i++) {
        lock_ordered();
    }
    return NULL;
}

int main(void) {
    pthread_t a;
    pthread_t b;

    arm_alarm();
    pthread_mutex_init(&gate.mu, NULL);
    pthread_cond_init(&gate.cv, NULL);
    gate.ready = 0;
    gate.go = 0;
    if (pthread_create(&a, NULL, worker, NULL) != 0 ||
        pthread_create(&b, NULL, worker, NULL) != 0) {
        perror("pthread_create");
        return 1;
    }
    pthread_mutex_lock(&gate.mu);
    while (gate.ready < 2) {
        pthread_cond_wait(&gate.cv, &gate.mu);
    }
    gate.go = 1;
    pthread_cond_broadcast(&gate.cv);
    pthread_mutex_unlock(&gate.mu);

    pthread_join(a, NULL);
    pthread_join(b, NULL);
    pthread_mutex_destroy(&mu_a);
    pthread_mutex_destroy(&mu_b);
    pthread_mutex_destroy(&gate.mu);
    pthread_cond_destroy(&gate.cv);
    printf("deadlock-fixed\n");
    return 0;
}
