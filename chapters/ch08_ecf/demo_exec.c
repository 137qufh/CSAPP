#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>

/* Replace the child with echo. Parent reaps the status. */
int main(void) {
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return 1;
    }
    if (pid == 0) {
        char *argv[] = {"echo", "echo-ok", NULL};
        execve("/bin/echo", argv, environ);
        execve("/usr/bin/echo", argv, environ);
        perror("execve");
        _exit(127);
    }

    int status = 0;
    if (waitpid(pid, &status, 0) < 0) {
        perror("waitpid");
        return 1;
    }
    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        return 1;
    }
    return 0;
}
