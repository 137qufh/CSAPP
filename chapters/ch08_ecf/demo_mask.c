#include <signal.h>
#include <stdio.h>
#include <string.h>

static volatile sig_atomic_t got;

static void on_usr1(int signo) {
    (void)signo;
    got = 1;
}

/* Block, raise, observe the pending set, then install a handler and unblock. */
int main(void) {
    sigset_t block;
    sigset_t old;
    sigset_t pending;
    sigemptyset(&block);
    sigaddset(&block, SIGUSR1);
    if (sigprocmask(SIG_BLOCK, &block, &old) != 0) {
        perror("sigprocmask");
        return 1;
    }
    if (raise(SIGUSR1) != 0) {
        perror("raise");
        sigprocmask(SIG_SETMASK, &old, NULL);
        return 1;
    }

    sigemptyset(&pending);
    if (sigpending(&pending) != 0) {
        perror("sigpending");
        sigprocmask(SIG_SETMASK, &old, NULL);
        return 1;
    }
    int is_pending = sigismember(&pending, SIGUSR1) == 1;
    printf("pending=%d\n", is_pending);
    fflush(stdout);

    struct sigaction sa;
    memset(&sa, 0, sizeof sa);
    sa.sa_handler = on_usr1;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGUSR1, &sa, NULL) != 0) {
        perror("sigaction");
        sigprocmask(SIG_SETMASK, &old, NULL);
        return 1;
    }
    if (sigprocmask(SIG_SETMASK, &old, NULL) != 0) {
        perror("sigprocmask");
        return 1;
    }
    if (!is_pending || !got) {
        return 1;
    }
    printf("delivered\n");
    return 0;
}
