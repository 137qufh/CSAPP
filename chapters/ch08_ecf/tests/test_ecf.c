#include "ex_forkcount.h"
#include "ex_signals.h"
#include "minitest.h"

#include <signal.h>
#include <string.h>

static volatile sig_atomic_t got_usr1;

static void on_usr1(int signo) {
    (void)signo;
    got_usr1 = 1;
}

int main(void) {
    MT_EQ_INT(pattern_processes(1), 8);
    MT_EQ_INT(pattern_processes(2), 3);
    MT_EQ_INT(pattern_processes(3), 5);
    MT_EQ_INT(pattern_processes(0), 0);
    MT_EQ_INT(pattern_processes(9), 0);

    MT_EQ_INT(usr1_became_pending(), 1);
    MT_EQ_INT(usr1_became_pending(), 1);

    sigset_t mask;
    sigemptyset(&mask);
    MT_EQ_INT(sigprocmask(SIG_SETMASK, NULL, &mask), 0);
    MT_EQ_INT(sigismember(&mask, SIGUSR1), 0);

    sigset_t pending;
    sigemptyset(&pending);
    MT_EQ_INT(sigpending(&pending), 0);
    MT_EQ_INT(sigismember(&pending, SIGUSR1), 0);

    struct sigaction sa;
    struct sigaction old;
    memset(&sa, 0, sizeof sa);
    sa.sa_handler = on_usr1;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    MT_EQ_INT(sigaction(SIGUSR1, &sa, &old), 0);
    got_usr1 = 0;
    MT_EQ_INT(raise(SIGUSR1), 0);
    MT_EQ_INT((int)got_usr1, 1);
    MT_EQ_INT(sigaction(SIGUSR1, &old, NULL), 0);

    MT_REPORT();
}
