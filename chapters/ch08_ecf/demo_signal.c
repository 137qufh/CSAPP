#include <signal.h>
#include <stdio.h>
#include <string.h>

static volatile sig_atomic_t got;

static void on_usr1(int signo) {
    (void)signo;
    got = 1;
}

/* Handler only stores a flag. main prints after raise returns. */
int main(void) {
    struct sigaction sa;
    memset(&sa, 0, sizeof sa);
    sa.sa_handler = on_usr1;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    if (sigaction(SIGUSR1, &sa, NULL) != 0) {
        perror("sigaction");
        return 1;
    }
    if (raise(SIGUSR1) != 0) {
        perror("raise");
        return 1;
    }
    if (!got) {
        return 1;
    }
    printf("caught\n");
    return 0;
}
