#include "my86.h"

#include <stdio.h>
#include <stdlib.h>

static char *read_file(const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) {
        perror(path);
        return NULL;
    }
    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        return NULL;
    }
    long n = ftell(f);
    if (n < 0) {
        fclose(f);
        return NULL;
    }
    rewind(f);
    char *b = malloc((size_t)n + 1);
    if (!b) {
        fclose(f);
        return NULL;
    }
    if (fread(b, 1, (size_t)n, f) != (size_t)n) {
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
    if (my86_run_seq(m, 1000000) != 0) {
        fprintf(stderr, "run: %s\n", m->err);
        return 1;
    }
    fputs(m->out, stdout);
    for (int i = 0; i < 8; i++) {
        printf("r%d=%u\n", i, m->reg[i]);
    }
    printf("steps=%llu\n", (unsigned long long)m->steps);
    free(src);
    my86_dispose(m);
    return 0;
}
