#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

static volatile sig_atomic_t got;

static void on_usr1(int signo) {
    (void)signo;
    got = 1;
}

/*
 * SIGUSR1 is blocked before the child is created, so a signal that
 * arrives early stays pending. The pipe tells the child that the
 * block is already in place. sigsuspend then atomically unblocks
 * and waits, which closes the window between "check the flag" and "sleep".
 */
int main(void) {
    sigset_t block;
    sigset_t old;
    sigemptyset(&block);
    sigaddset(&block, SIGUSR1);
    if (sigprocmask(SIG_BLOCK, &block, &old) != 0) {
        perror("sigprocmask");
        return 1;
    }

    struct sigaction sa;
    memset(&sa, 0, sizeof sa);
    sa.sa_handler = on_usr1;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGUSR1, &sa, NULL) != 0) {
        perror("sigaction");
        return 1;
    }

    int fds[2];
    if (pipe(fds) != 0) {
        perror("pipe");
        return 1;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return 1;
    }
    if (pid == 0) {
        close(fds[1]);
        char token = 0;
        ssize_t n = read(fds[0], &token, 1);
        close(fds[0]);
        if (n != 1) {
            _exit(1);
        }
        if (kill(getppid(), SIGUSR1) != 0) {
            _exit(1);
        }
        _exit(0);
    }

    close(fds[0]);
    char ready = 'B';
    if (write(fds[1], &ready, 1) != 1) {
        perror("write");
        close(fds[1]);
        waitpid(pid, NULL, 0);
        return 1;
    }
    close(fds[1]);

    while (!got) {
        sigsuspend(&old);
    }
    printf("resumed\n");

    int status = 0;
    if (waitpid(pid, &status, 0) < 0) {
        perror("waitpid");
        return 1;
    }
    if (sigprocmask(SIG_SETMASK, &old, NULL) != 0) {
        perror("sigprocmask");
        return 1;
    }
    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        return 1;
    }
    return 0;
}
