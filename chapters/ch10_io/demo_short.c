/* Move a large buffer across a pipe.
 *
 * read() and write() may transfer fewer bytes than requested. That short
 * count is normal: a pipe has a finite kernel buffer, a signal can interrupt
 * the call, and a disk write can stop at a device boundary. One call is not
 * a complete transfer. sc_readn / sc_writen loop until all n bytes move, or
 * until EOF or a real error.
 */

#include "sc_bio.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#define NBYTES 100000

int main(void) {
    int sp[2];
    if (pipe(sp) < 0) {
        perror("pipe");
        return 1;
    }

    fflush(NULL);
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        close(sp[0]);
        close(sp[1]);
        return 1;
    }

    if (pid == 0) {
        unsigned char *buf = malloc(NBYTES);
        ssize_t n;
        int ok = 0;
        int i;
        close(sp[1]);
        if (!buf) {
            close(sp[0]);
            _exit(1);
        }
        n = sc_readn(sp[0], buf, NBYTES);
        ok = (n == NBYTES);
        for (i = 0; ok && i < NBYTES; i++) {
            if (buf[i] != (unsigned char)(i * 3)) {
                ok = 0;
            }
        }
        free(buf);
        close(sp[0]);
        _exit(ok ? 0 : 1);
    }

    unsigned char *buf = malloc(NBYTES);
    ssize_t n;
    int status = 0;
    int i;
    close(sp[0]);
    if (!buf) {
        close(sp[1]);
        waitpid(pid, &status, 0);
        return 1;
    }
    for (i = 0; i < NBYTES; i++) {
        buf[i] = (unsigned char)(i * 3);
    }
    n = sc_writen(sp[1], buf, NBYTES);
    free(buf);
    close(sp[1]);
    if (waitpid(pid, &status, 0) < 0) {
        perror("waitpid");
        return 1;
    }
    if (n != NBYTES || !WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        fprintf(stderr, "short transfer failed\n");
        return 1;
    }
    printf("short-ok\n");
    return 0;
}
