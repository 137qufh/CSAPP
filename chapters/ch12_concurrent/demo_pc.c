/* Bounded buffer. empty/full semaphores count slots; a mutex covers the indices.
 * 1000 produced values must sum to the same checksum the consumer observes.
 */

#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define CAP 8
#define NITEM 1000

static int slots[CAP];
static int head = 0;
static int tail = 0;
static sem_t sem_empty;
static sem_t sem_full;
static sem_t sem_mu;

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

static void sem_wait_retry(sem_t *s) {
    while (sem_wait(s) < 0) {
        if (errno != EINTR) {
            perror("sem_wait");
            _exit(1);
        }
    }
}

static void *producer(void *arg) {
    int i;
    (void)arg;
    for (i = 1; i <= NITEM; i++) {
        sem_wait_retry(&sem_empty);
        sem_wait_retry(&sem_mu);
        slots[tail] = i;
        tail = (tail + 1) % CAP;
        if (sem_post(&sem_mu) < 0 || sem_post(&sem_full) < 0) {
            perror("sem_post");
            _exit(1);
        }
    }
    return NULL;
}

static void *consumer(void *arg) {
    long long *sum = arg;
    long long acc = 0;
    int i;
    for (i = 0; i < NITEM; i++) {
        int value;
        sem_wait_retry(&sem_full);
        sem_wait_retry(&sem_mu);
        value = slots[head];
        head = (head + 1) % CAP;
        if (sem_post(&sem_mu) < 0 || sem_post(&sem_empty) < 0) {
            perror("sem_post");
            _exit(1);
        }
        acc += value;
    }
    *sum = acc;
    return NULL;
}

int main(void) {
    pthread_t tp;
    pthread_t tc;
    long long sum = 0;
    long long expect = (long long)NITEM * (NITEM + 1) / 2;

    arm_alarm();
    if (sem_init(&sem_empty, 0, CAP) < 0 || sem_init(&sem_full, 0, 0) < 0 ||
        sem_init(&sem_mu, 0, 1) < 0) {
        perror("sem_init");
        return 1;
    }
    if (pthread_create(&tp, NULL, producer, NULL) != 0 ||
        pthread_create(&tc, NULL, consumer, &sum) != 0) {
        perror("pthread_create");
        return 1;
    }
    pthread_join(tp, NULL);
    pthread_join(tc, NULL);
    sem_destroy(&sem_empty);
    sem_destroy(&sem_full);
    sem_destroy(&sem_mu);
    printf("checksum=%lld\n", sum);
    if (sum != expect) {
        fprintf(stderr, "checksum mismatch\n");
        return 1;
    }
    printf("pc-ok\n");
    return 0;
}
