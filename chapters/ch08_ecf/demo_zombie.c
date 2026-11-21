#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

/*
 * The child exits while the parent has not yet waited, so the kernel
 * keeps a zombie until waitpid. We look at /proc after a short pause,
 * then always reap. A missing /proc is not a failure of the demo.
 */
int main(void) {
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return 1;
    }
    if (pid == 0) {
        _exit(0);
    }

    if (usleep(20000) != 0) {
        /* A short pause is enough; keep going if it was interrupted. */
    }

    char path[64];
    snprintf(path, sizeof path, "/proc/%d/status", (int)pid);
    FILE *fp = fopen(path, "r");
    if (!fp) {
        printf("proc-unavailable\n");
    } else {
        char line[256];
        int saw = 0;
        int is_z = 0;
        while (fgets(line, sizeof line, fp)) {
            if (strncmp(line, "State:", 6) == 0) {
                saw = 1;
                if (strchr(line, 'Z') != NULL) {
                    is_z = 1;
                }
                break;
            }
        }
        fclose(fp);
        if (!saw) {
            printf("proc-unavailable\n");
        } else if (is_z) {
            printf("state=Z\n");
        } else {
            printf("state=not-Z\n");
        }
    }

    int status = 0;
    if (waitpid(pid, &status, 0) < 0) {
        perror("waitpid");
        return 1;
    }
    return 0;
}
