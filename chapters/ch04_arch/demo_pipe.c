#include "my86.h"

#include <stdio.h>
#include <stdlib.h>

static char *read_file(const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) {
        perror(path);
        return NULL;
    }
    fseek(f, 0, SEEK_END);
    long n = ftell(f);
    rewind(f);
    char *b = malloc((size_t)n + 1);
    if (!b || fread(b, 1, (size_t)n, f) != (size_t)n) {
        free(b);
        fclose(f);
        return NULL;
    }
    b[n] = '\0';
    fclose(f);
    return b;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "usage: %s program.my\n", argv[0]);
        return 1;
    }
    char *src = read_file(argv[1]);
    if (!src) {
        return 1;
    }
    my86_t *m = my86_new();
    if (my86_assemble(m, src) != 0) {
        fprintf(stderr, "assemble: %s\n", m->err);
        return 1;
    }
    if (my86_run_pipe(m, 1000000, 1) != 0) {
        fprintf(stderr, "pipe: %s\n", m->err);
        return 1;
    }
    printf("retired=%llu cycles=%llu stalls=%d flushes=%d forwards=%d\n",
           (unsigned long long)m->steps, (unsigned long long)m->cycles,
           m->pipe_stalls, m->pipe_flushes, m->pipe_forwards);
    fputs(m->out, stdout);
    free(src);
    my86_dispose(m);
    return 0;
}
