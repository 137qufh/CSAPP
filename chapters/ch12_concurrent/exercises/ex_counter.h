#ifndef EX_COUNTER_H
#define EX_COUNTER_H

#include <pthread.h>

typedef struct {
    int value;
    pthread_mutex_t mu;
} safe_counter_t;

void sc_init(safe_counter_t *c);
void sc_add(safe_counter_t *c, int d);
int sc_get(safe_counter_t *c);
void sc_destroy(safe_counter_t *c);

#endif
