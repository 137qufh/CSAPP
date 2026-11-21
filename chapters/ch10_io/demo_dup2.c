/* Child dup2's a file onto stdout, then execs /bin/echo into that file. */

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void) {
    char path[] = "/tmp/scraft-dup2-XXXXXX";
    char buf[64];
    ssize_t n;
    int fd;
    int status = 0;
    pid_t pid;

    fd = mkstemp(path);
    if (fd < 0) {
        perror("mkstemp");
        return 1;
    }
    if (close(fd) < 0) {
        perror("close");
        unlink(path);
        return 1;
    }

    fflush(NULL);
    pid = fork();
    if (pid < 0) {
        perror("fork");
        unlink(path);
        return 1;
    }
    if (pid == 0) {
        int out = open(path, O_WRONLY | O_TRUNC);
        if (out < 0) {
            _exit(1);
        }
        if (dup2(out, STDOUT_FILENO) < 0) {
            _exit(1);
        }
        if (out != STDOUT_FILENO && close(out) < 0) {
            _exit(1);
        }
        execl("/bin/echo", "echo", "dup2-line", (char *)NULL);
        _exit(127);
    }

    if (waitpid(pid, &status, 0) < 0) {
        perror("waitpid");
        unlink(path);
        return 1;
    }
    fd = open(path, O_RDONLY);
    if (fd < 0) {
        perror("open");
        unlink(path);
        return 1;
    }
    n = read(fd, buf, sizeof(buf) - 1);
    close(fd);
    unlink(path);
    if (n < 0) {
        perror("read");
        return 1;
    }
    buf[n] = '\0';
    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0 ||
        strcmp(buf, "dup2-line\n") != 0) {
        fprintf(stderr, "dup2 redirect failed: [%s]\n", buf);
        return 1;
    }
    printf("dup2-ok\n");
    return 0;
}
