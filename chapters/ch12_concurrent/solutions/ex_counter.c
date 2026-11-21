#include "ex_counter.h"

void sc_init(safe_counter_t *c) {
    c->value = 0;
    pthread_mutex_init(&c->mu, NULL);
}

void sc_add(safe_counter_t *c, int d) {
    pthread_mutex_lock(&c->mu);
    c->value += d;
    pthread_mutex_unlock(&c->mu);
}

int sc_get(safe_counter_t *c) {
    int v;
    pthread_mutex_lock(&c->mu);
    v = c->value;
    pthread_mutex_unlock(&c->mu);
    return v;
}

void sc_destroy(safe_counter_t *c) {
    pthread_mutex_destroy(&c->mu);
}
