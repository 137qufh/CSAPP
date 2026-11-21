#include "eval.h"
#include "parse.h"

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAXJOBS 32

typedef struct {
    int used;
    int id;
    pid_t pid;
    int done;
    char text[160];
} job_t;

static job_t jobs[MAXJOBS];
static int next_id = 1;
static volatile pid_t fg_pid = 0;

static void trim_copy(char *dst, size_t n, const char *src) {
    while (*src && (*src == ' ' || *src == '\t')) {
        src++;
    }
    snprintf(dst, n, "%s", src);
    size_t L = strlen(dst);
    while (L && (dst[L - 1] == '\n' || dst[L - 1] == '\r' || dst[L - 1] == ' ')) {
        dst[--L] = '\0';
    }
}

static void on_chld(int sig) {
    (void)sig;
    int saved = errno;
    int status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        for (int i = 0; i < MAXJOBS; i++) {
            if (jobs[i].used && jobs[i].pid == pid) {
                jobs[i].done = 1;
            }
        }
    }
    errno = saved;
}

static void on_int(int sig) {
    (void)sig;
    pid_t p = fg_pid;
    if (p > 0) {
        kill(p, SIGINT);
    }
}

void shell_init(void) {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = on_chld;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sigaction(SIGCHLD, &sa, NULL);
    sa.sa_handler = on_int;
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);
    signal(SIGTSTP, SIG_IGN);
}

static job_t *add_job(pid_t pid, const char *text) {
    for (int i = 0; i < MAXJOBS; i++) {
        if (!jobs[i].used || jobs[i].done) {
            jobs[i].used = 1;
            jobs[i].done = 0;
            jobs[i].id = next_id++;
            jobs[i].pid = pid;
            trim_copy(jobs[i].text, sizeof(jobs[i].text), text);
            return &jobs[i];
        }
    }
    return NULL;
}

static job_t *find_job(int id) {
    for (int i = 0; i < MAXJOBS; i++) {
        if (jobs[i].used && jobs[i].id == id) {
            return &jobs[i];
        }
    }
    return NULL;
}

static void builtin_jobs(void) {
    for (int i = 0; i < MAXJOBS; i++) {
        if (jobs[i].used && !jobs[i].done) {
            printf("[%d] running %s\n", jobs[i].id, jobs[i].text);
        }
    }
    fflush(stdout);
}

static int argv_is(const sc_cmd_t *c, const char *s) {
    return c->argc > 0 && strcmp(c->argv[0], s) == 0;
}

static void run_builtin(const sc_cmd_t *c) {
    if (argv_is(c, "quit") || argv_is(c, "exit")) {
        exit(0);
    }
    if (argv_is(c, "cd")) {
        const char *dir = c->argc > 1 ? c->argv[1] : getenv("HOME");
        if (!dir || chdir(dir) < 0) {
            perror("cd");
        }
        return;
    }
    if (argv_is(c, "jobs")) {
        builtin_jobs();
        return;
    }
    if (argv_is(c, "fg") || argv_is(c, "bg")) {
        int id = 0;
        if (c->argc > 1) {
            const char *a = c->argv[1];
            if (*a == '%') {
                a++;
            }
            id = atoi(a);
        }
        job_t *j = find_job(id);
        if (!j || j->done) {
            fprintf(stderr, "no such job\n");
            return;
        }
        kill(j->pid, SIGCONT);
        if (argv_is(c, "fg")) {
            fg_pid = j->pid;
            int status;
            if (waitpid(j->pid, &status, 0) > 0) {
                j->done = 1;
            }
            fg_pid = 0;
        }
    }
}

static int launch(sc_line_t *p, pid_t *pids) {
    int prev = -1;
    for (int i = 0; i < p->ncmds; i++) {
        int fd[2] = {-1, -1};
        if (i + 1 < p->ncmds && pipe(fd) < 0) {
            perror("pipe");
            return -1;
        }
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            return -1;
        }
        if (pid == 0) {
            signal(SIGINT, SIG_DFL);
            signal(SIGCHLD, SIG_DFL);
            signal(SIGTSTP, SIG_DFL);
            if (prev >= 0) {
                dup2(prev, STDIN_FILENO);
                close(prev);
            }
            if (fd[1] >= 0) {
                dup2(fd[1], STDOUT_FILENO);
                close(fd[1]);
                close(fd[0]);
            }
            sc_cmd_t *c = &p->cmds[i];
            if (c->infile) {
                int in = open(c->infile, O_RDONLY);
                if (in < 0) {
                    perror(c->infile);
                    _exit(1);
                }
                dup2(in, STDIN_FILENO);
                close(in);
            }
            if (c->outfile) {
                int flags = O_WRONLY | O_CREAT | (c->append ? O_APPEND : O_TRUNC);
                int out = open(c->outfile, flags, 0644);
                if (out < 0) {
                    perror(c->outfile);
                    _exit(1);
                }
                dup2(out, STDOUT_FILENO);
                close(out);
            }
            execvp(c->argv[0], c->argv);
            perror(c->argv[0]);
            _exit(127);
        }
        pids[i] = pid;
        if (prev >= 0) {
            close(prev);
        }
        if (fd[1] >= 0) {
            close(fd[1]);
        }
        prev = fd[0];
    }
    if (prev >= 0) {
        close(prev);
    }
    return 0;
}

void eval_line(const char *line) {
    sc_line_t parsed;
    if (sc_parse(line, &parsed) != 0) {
        fprintf(stderr, "parse error\n");
        sc_line_free(&parsed);
        return;
    }
    if (parsed.ncmds == 0) {
        sc_line_free(&parsed);
        return;
    }
    int single_bi = parsed.ncmds == 1 && !parsed.background &&
                    (argv_is(&parsed.cmds[0], "quit") || argv_is(&parsed.cmds[0], "exit") ||
                     argv_is(&parsed.cmds[0], "cd") || argv_is(&parsed.cmds[0], "jobs") ||
                     argv_is(&parsed.cmds[0], "fg") || argv_is(&parsed.cmds[0], "bg"));
    if (single_bi) {
        run_builtin(&parsed.cmds[0]);
        sc_line_free(&parsed);
        return;
    }

    sigset_t block, prevmask;
    sigemptyset(&block);
    sigaddset(&block, SIGCHLD);
    sigprocmask(SIG_BLOCK, &block, &prevmask);

    pid_t pids[SC_MAXCMDS];
    if (launch(&parsed, pids) < 0) {
        sigprocmask(SIG_SETMASK, &prevmask, NULL);
        sc_line_free(&parsed);
        return;
    }
    if (parsed.background) {
        job_t *j = add_job(pids[0], parsed.text);
        sigprocmask(SIG_SETMASK, &prevmask, NULL);
        if (j) {
            printf("[%d] %d\n", j->id, (int)j->pid);
            fflush(stdout);
        }
    } else {
        fg_pid = pids[parsed.ncmds - 1];
        sigprocmask(SIG_SETMASK, &prevmask, NULL);
        for (int i = 0; i < parsed.ncmds; i++) {
            int status;
            while (waitpid(pids[i], &status, 0) < 0 && errno == EINTR) {
            }
        }
        fg_pid = 0;
    }
    sc_line_free(&parsed);
}
