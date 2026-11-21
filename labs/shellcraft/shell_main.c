#include "eval.h"

#include <stdio.h>
#include <unistd.h>

int main(void) {
    shell_init();
    int tty = isatty(STDIN_FILENO);
    char buf[1024];
    while (1) {
        if (tty) {
            fputs("scsh> ", stdout);
            fflush(stdout);
        }
        if (!fgets(buf, sizeof(buf), stdin)) {
            break;
        }
        eval_line(buf);
    }
    return 0;
}
