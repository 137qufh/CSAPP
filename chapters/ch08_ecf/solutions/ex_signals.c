#include "ex_signals.h"

#include <signal.h>

int usr1_became_pending(void) {
    sigset_t extra;
    sigset_t old;
    sigset_t pending;
    sigemptyset(&extra);
    sigaddset(&extra, SIGUSR1);
    if (sigprocmask(SIG_BLOCK, &extra, &old) != 0) {
        return 0;
    }
    if (raise(SIGUSR1) != 0) {
        sigprocmask(SIG_SETMASK, &old, NULL);
        return 0;
    }

    sigemptyset(&pending);
    int saw = 0;
    if (sigpending(&pending) == 0 && sigismember(&pending, SIGUSR1) == 1) {
        saw = 1;
    }

    /* sigwait removes one pending instance while the signal stays blocked.
       The previous disposition is left alone, so a later raise still delivers. */
    int signo = 0;
    int waited = sigwait(&extra, &signo);
    if (sigprocmask(SIG_SETMASK, &old, NULL) != 0) {
        return 0;
    }
    if (waited != 0 || signo != SIGUSR1) {
        return 0;
    }

    sigemptyset(&pending);
    if (sigpending(&pending) != 0 || sigismember(&pending, SIGUSR1) == 1) {
        return 0;
    }
    return saw;
}
