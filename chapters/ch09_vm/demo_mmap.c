#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>

/* MAP_PRIVATE is copy-on-write. The child store must not show up in the parent. */
int main(void) {
    size_t n = 4096;
    char *p = mmap(NULL, n, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (p == MAP_FAILED) {
        perror("mmap");
        return 1;
    }
    memcpy(p, "parent", 7);
    int fds[2];
    if (pipe(fds) < 0) {
        perror("pipe");
        return 1;
    }
    pid_t c = fork();
    if (c < 0) {
        perror("fork");
        return 1;
    }
    if (c == 0) {
        memcpy(p, "child!", 7);
        if (write(fds[1], p, 6) < 0) {
            _exit(1);
        }
        _exit(0);
    }
    char buf[8] = {0};
    if (read(fds[0], buf, 6) != 6) {
        return 1;
    }
    waitpid(c, NULL, 0);
    printf("parent-sees=%s child-wrote=%s\n", p, buf);
    if (strcmp(p, "parent") != 0 || strcmp(buf, "child!") != 0) {
        return 1;
    }
    munmap(p, n);
    printf("cow-ok\n");
    return 0;
}
