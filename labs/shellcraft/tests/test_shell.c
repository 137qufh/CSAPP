#include "minitest.h"
#include "parse.h"

#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

static void test_parse(void) {
    sc_line_t p;
    MT_EQ_INT(sc_parse("  /bin/echo hello  \n", &p), 0);
    MT_EQ_INT(p.ncmds, 1);
    MT_EQ_INT(p.background, 0);
    MT_EQ_STR(p.cmds[0].argv[0], "/bin/echo");
    MT_EQ_STR(p.cmds[0].argv[1], "hello");
    sc_line_free(&p);

    MT_EQ_INT(sc_parse("/bin/echo a | /bin/cat > out &\n", &p), 0);
    MT_EQ_INT(p.ncmds, 2);
    MT_EQ_INT(p.background, 1);
    MT_EQ_STR(p.cmds[1].outfile, "out");
    MT_EQ_STR(p.cmds[0].argv[1], "a");
    sc_line_free(&p);

    MT_EQ_INT(sc_parse("/bin/cat < in >> log\n", &p), 0);
    MT_EQ_STR(p.cmds[0].infile, "in");
    MT_EQ_STR(p.cmds[0].outfile, "log");
    MT_EQ_INT(p.cmds[0].append, 1);
    sc_line_free(&p);
}

static int run_shell(const char *bin, const char *script, char *out, size_t outn) {
    int in[2], ou[2];
    if (pipe(in) < 0 || pipe(ou) < 0) {
        return -1;
    }
    pid_t pid = fork();
    if (pid < 0) {
        return -1;
    }
    if (pid == 0) {
        dup2(in[0], STDIN_FILENO);
        dup2(ou[1], STDOUT_FILENO);
        dup2(ou[1], STDERR_FILENO);
        close(in[0]);
        close(in[1]);
        close(ou[0]);
        close(ou[1]);
        execl(bin, bin, (char *)NULL);
        _exit(127);
    }
    close(in[0]);
    close(ou[1]);
    if (write(in[1], script, strlen(script)) < 0) {
        return -1;
    }
    close(in[1]);
    size_t got = 0;
    while (got + 1 < outn) {
        ssize_t r = read(ou[0], out + got, outn - 1 - got);
        if (r < 0) {
            break;
        }
        if (r == 0) {
            break;
        }
        got += (size_t)r;
    }
    out[got] = '\0';
    close(ou[0]);
    alarm(8);
    int status = 0;
    waitpid(pid, &status, 0);
    alarm(0);
    return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
}

static void test_exec(const char *bin) {
    char path[64];
    snprintf(path, sizeof(path), "/tmp/scsh-%d", getpid());
    unlink(path);
    char script[512];
    snprintf(script, sizeof(script),
             "/bin/echo hello\n"
             "/bin/echo filedata > %s\n"
             "/bin/echo foo | /bin/cat\n"
             "cd /tmp\n"
             "/bin/pwd\n"
             "/bin/echo still\n"
             "quit\n",
             path);
    char out[1024];
    int rc = run_shell(bin, script, out, sizeof(out));
    MT_EQ_INT(rc, 0);
    MT_ASSERT(strstr(out, "hello\n") != NULL);
    MT_ASSERT(strstr(out, "foo\n") != NULL);
    MT_ASSERT(strstr(out, "/tmp\n") != NULL);
    MT_ASSERT(strstr(out, "still\n") != NULL);
    FILE *f = fopen(path, "r");
    MT_ASSERT(f != NULL);
    char body[64] = {0};
    if (f) {
        if (!fgets(body, sizeof(body), f)) {
            body[0] = '\0';
        }
        fclose(f);
    }
    MT_EQ_STR(body, "filedata\n");
    unlink(path);
}

int main(int argc, char **argv) {
    signal(SIGALRM, SIG_DFL);
    test_parse();
    if (argc > 1) {
        test_exec(argv[1]);
    }
    MT_REPORT();
}
