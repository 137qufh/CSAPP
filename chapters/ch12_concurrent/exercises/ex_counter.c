#include "ex_counter.h"

void sc_init(safe_counter_t *c) {
    c->value = 0;
    pthread_mutex_init(&c->mu, NULL);
}

void sc_add(safe_counter_t *c, int d) {
    (void)c;
    (void)d;
    /* TODO: add d while holding the mutex. */
}

int sc_get(safe_counter_t *c) {
    (void)c;
    /* TODO: return the current value while holding the mutex. */
    return 0;
}

void sc_destroy(safe_counter_t *c) {
    pthread_mutex_destroy(&c->mu);
}
