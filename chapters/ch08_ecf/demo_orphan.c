#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>

/*
 * A true orphan appears when the parent exits first. The kernel then
 * reparents the child to init (pid 1) or to a child subreaper. That
 * sequence races with a parent that must itself be reaped by make, so
 * this demo does not exit early.
 *
 * Instead the parent stays alive, forks a child, waits, then forks a
 * second child. Each child prints the pid it observes. Both should
 * still name this process. Nobody is left unreaped.
 */
int main(void) {
    pid_t parent = getpid();
    printf("parent-pid=%d\n", (int)parent);
    fflush(stdout);

    for (int i = 0; i < 2; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            return 1;
        }
        if (pid == 0) {
            pid_t seen = getppid();
            printf("child-sees=%d\n", (int)seen);
            fflush(stdout);
            _exit(seen == parent ? 0 : 2);
        }

        int status = 0;
        if (waitpid(pid, &status, 0) < 0) {
            perror("waitpid");
            return 1;
        }
        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
            return 1;
        }
    }
    return 0;
}
