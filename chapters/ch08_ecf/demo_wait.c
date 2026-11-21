#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>

/* Two children, two distinct statuses, reaped in creation order. */
int main(void) {
    const int codes[2] = {3, 5};
    pid_t pids[2];

    for (int i = 0; i < 2; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            return 1;
        }
        if (pid == 0) {
            _exit(codes[i]);
        }
        pids[i] = pid;
    }

    for (int i = 0; i < 2; i++) {
        int status = 0;
        if (waitpid(pids[i], &status, 0) < 0) {
            perror("waitpid");
            return 1;
        }
        if (!WIFEXITED(status)) {
            return 1;
        }
        printf("code=%d\n", WEXITSTATUS(status));
    }
    return 0;
}
