/* Drive ./defuse. With no arguments, check a bad line and the
 * instructor answer file. With one argument, that file must clear
 * every phase (the student target). A stuck child is killed after
 * five seconds.
 */
#include "minitest.h"

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

static volatile sig_atomic_t g_timed_out;

static void on_alrm(int sig) {
    (void)sig;
    g_timed_out = 1;
}

struct run_result {
    int exit_code;
    int timed_out;
    char out[8192];
};

static int write_all(int fd, const char *buf, size_t n) {
    size_t off = 0;
    while (off < n) {
        ssize_t w = write(fd, buf + off, n - off);
        if (w < 0) {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }
        off += (size_t)w;
    }
    return 0;
}

static int run_capture(const char *bin, const char *input, struct run_result *rr) {
    int in_fd[2] = {-1, -1};
    int out_fd[2] = {-1, -1};

    rr->exit_code = -1;
    rr->timed_out = 0;
    rr->out[0] = '\0';

    if (pipe(in_fd) != 0) {
        return -1;
    }
    if (pipe(out_fd) != 0) {
        close(in_fd[0]);
        close(in_fd[1]);
        return -1;
    }

    pid_t pid = fork();
    if (pid < 0) {
        close(in_fd[0]);
        close(in_fd[1]);
        close(out_fd[0]);
        close(out_fd[1]);
        return -1;
    }
    if (pid == 0) {
        if (dup2(in_fd[0], STDIN_FILENO) < 0 || dup2(out_fd[1], STDOUT_FILENO) < 0) {
            _exit(127);
        }
        close(in_fd[0]);
        close(in_fd[1]);
        close(out_fd[0]);
        close(out_fd[1]);
        execl(bin, bin, (char *)NULL);
        _exit(127);
    }

    close(in_fd[0]);
    close(out_fd[1]);
    if (input != NULL) {
        (void)write_all(in_fd[1], input, strlen(input));
    }
    close(in_fd[1]);

    g_timed_out = 0;
    struct sigaction sa;
    memset(&sa, 0, sizeof sa);
    sa.sa_handler = on_alrm;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGALRM, &sa, NULL);
    alarm(5);

    size_t used = 0;
    for (;;) {
        if (g_timed_out || used + 1 >= sizeof rr->out) {
            break;
        }
        ssize_t r = read(out_fd[0], rr->out + used, sizeof rr->out - 1 - used);
        if (r < 0) {
            if (errno == EINTR && !g_timed_out) {
                continue;
            }
            break;
        }
        if (r == 0) {
            break;
        }
        used += (size_t)r;
        rr->out[used] = '\0';
    }
    alarm(0);
    close(out_fd[0]);

    if (g_timed_out) {
        kill(pid, SIGKILL);
        rr->timed_out = 1;
    }

    int st = 0;
    for (;;) {
        pid_t w = waitpid(pid, &st, 0);
        if (w < 0) {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }
        break;
    }

    if (rr->timed_out || !WIFEXITED(st)) {
        rr->exit_code = -1;
    } else {
        rr->exit_code = WEXITSTATUS(st);
    }
    return 0;
}

static char *read_file(const char *path) {
    FILE *fp = fopen(path, "r");
    if (fp == NULL) {
        return NULL;
    }
    if (fseek(fp, 0, SEEK_END) != 0) {
        fclose(fp);
        return NULL;
    }
    long n = ftell(fp);
    if (n < 0) {
        fclose(fp);
        return NULL;
    }
    if (fseek(fp, 0, SEEK_SET) != 0) {
        fclose(fp);
        return NULL;
    }
    char *buf = malloc((size_t)n + 1);
    if (buf == NULL) {
        fclose(fp);
        return NULL;
    }
    size_t got = fread(buf, 1, (size_t)n, fp);
    buf[got] = '\0';
    fclose(fp);
    return buf;
}

static void expect_cleared(const char *input) {
    struct run_result rr;
    MT_ASSERT(run_capture("./defuse", input, &rr) == 0);
    MT_ASSERT(!rr.timed_out);
    MT_EQ_INT(rr.exit_code, 0);
    MT_ASSERT(strstr(rr.out, "phase 1 cleared") != NULL);
    MT_ASSERT(strstr(rr.out, "phase 2 cleared") != NULL);
    MT_ASSERT(strstr(rr.out, "phase 3 cleared") != NULL);
    MT_ASSERT(strstr(rr.out, "phase 4 cleared") != NULL);
    MT_ASSERT(strstr(rr.out, "phase 5 cleared") != NULL);
    MT_ASSERT(strstr(rr.out, "all phases cleared") != NULL);
}

int main(int argc, char **argv) {
    signal(SIGPIPE, SIG_IGN);

    if (argc >= 2) {
        char *text = read_file(argv[1]);
        MT_ASSERT(text != NULL);
        if (text != NULL) {
            expect_cleared(text);
            free(text);
        }
        MT_REPORT();
    }

    struct run_result rr;
    MT_ASSERT(run_capture("./defuse", "not-a-phase\n", &rr) == 0);
    MT_ASSERT(!rr.timed_out);
    MT_ASSERT(rr.exit_code != 0);
    MT_ASSERT(strstr(rr.out, "phase 1 failed") != NULL);
    MT_ASSERT(strstr(rr.out, "all phases cleared") == NULL);

    char *answers = read_file("solutions/answers.txt");
    MT_ASSERT(answers != NULL);
    if (answers != NULL) {
        expect_cleared(answers);
        free(answers);
    }
    MT_REPORT();
}
