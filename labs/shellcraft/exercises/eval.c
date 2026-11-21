#include "eval.h"

void shell_init(void) { /* TODO: install SIGCHLD / SIGINT handlers if you add job control. */ }

void eval_line(const char *line) {
    (void)line;
    /* TODO: parse, run builtins (quit, cd, jobs, fg, bg), fork/exec pipelines,
     * wire up pipes and < > >>, and track background jobs.
     */
}
