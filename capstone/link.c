#include "link.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXSYM 256
#define MAXEXP 256

typedef struct {
    char name[64];
    int file;
    int global;
} sym_t;

static sym_t syms[MAXSYM];
static int nsyms;
static char exports[16][64][MAXEXP];
static int nexp[16];

static int is_id(const char *s) {
    if (!s || !(isalpha((unsigned char)s[0]) || s[0] == '_' || s[0] == '.')) {
        return 0;
    }
    if (s[0] == '.') {
        return 0;
    }
    for (const char *p = s; *p; p++) {
        if (!(isalnum((unsigned char)*p) || *p == '_')) {
            return 0;
        }
    }
    return 1;
}

static int is_reg(const char *s) {
    if (!strcmp(s, "lr") || !strcmp(s, "sp") || !strcmp(s, "zero")) {
        return 1;
    }
    return s[0] == 'r' && s[1] >= '0' && s[1] <= '7' && s[2] == '\0';
}

static sym_t *find_sym(const char *name) {
    for (int i = 0; i < nsyms; i++) {
        if (strcmp(syms[i].name, name) == 0) {
            return &syms[i];
        }
    }
    return NULL;
}

static int add_sym(const char *name, int file, int global, char *err, int errlen) {
    sym_t *old = find_sym(name);
    if (old) {
        if (old->global || global) {
            snprintf(err, (size_t)errlen, "duplicate symbol %s", name);
            return -1;
        }
        if (old->file == file) {
            snprintf(err, (size_t)errlen, "duplicate label %s", name);
            return -1;
        }
        /* same local name in another file is fine; they are prefixed later */
    }
    if (old && !old->global && !global && old->file != file) {
        /* keep both locals */
    }
    if (nsyms >= MAXSYM) {
        snprintf(err, (size_t)errlen, "too many symbols");
        return -1;
    }
    if (old && !old->global && !global) {
        /* fall through and store another local with the same name */
    }
    snprintf(syms[nsyms].name, sizeof(syms[0].name), "%s", name);
    syms[nsyms].file = file;
    syms[nsyms].global = global;
    nsyms++;
    return 0;
}

static sym_t *find_visible(const char *name, int file) {
    sym_t *local = NULL;
    sym_t *global = NULL;
    for (int i = 0; i < nsyms; i++) {
        if (strcmp(syms[i].name, name) != 0) {
            continue;
        }
        if (syms[i].global) {
            global = &syms[i];
        } else if (syms[i].file == file) {
            local = &syms[i];
        }
    }
    return local ? local : global;
}

static void rename_sym(const sym_t *s, char *out, size_t n) {
    if (s->global) {
        snprintf(out, n, "%s", s->name);
    } else {
        snprintf(out, n, "f%d_%s", s->file, s->name);
    }
}

static void strip_comment(char *s) {
    for (char *p = s; *p; p++) {
        if (*p == '#') {
            *p = '\0';
            return;
        }
    }
}

static char *trim(char *s) {
    while (*s && isspace((unsigned char)*s)) {
        s++;
    }
    char *e = s + strlen(s);
    while (e > s && isspace((unsigned char)e[-1])) {
        *--e = '\0';
    }
    return s;
}

static int split_tok(char *s, char **tok, int maxn) {
    int n = 0;
    while (*s && n < maxn) {
        while (*s && (isspace((unsigned char)*s) || *s == ',')) {
            s++;
        }
        if (!*s) {
            break;
        }
        tok[n++] = s;
        while (*s && !isspace((unsigned char)*s) && *s != ',') {
            s++;
        }
        if (*s) {
            *s++ = '\0';
        }
    }
    return n;
}

static int exported(int file, const char *name) {
    for (int i = 0; i < nexp[file]; i++) {
        if (strcmp(exports[file][i], name) == 0) {
            return 1;
        }
    }
    return 0;
}

char *link_modules(char **sources, int nfiles, char *err, int errlen) {
    nsyms = 0;
    memset(nexp, 0, sizeof(nexp));
    if (nfiles > 16) {
        snprintf(err, (size_t)errlen, "too many modules");
        return NULL;
    }
    for (int f = 0; f < nfiles; f++) {
        char *copy = strdup(sources[f]);
        if (!copy) {
            return NULL;
        }
        char *save = NULL;
        for (char *line = strtok_r(copy, "\n", &save); line; line = strtok_r(NULL, "\n", &save)) {
            strip_comment(line);
            line = trim(line);
            if (!*line) {
                continue;
            }
            char buf[256];
            snprintf(buf, sizeof(buf), "%s", line);
            char *tok[8];
            int nt = split_tok(buf, tok, 8);
            if (nt >= 2 && (strcmp(tok[0], ".global") == 0 || strcmp(tok[0], ".globl") == 0)) {
                if (nexp[f] < MAXEXP) {
                    snprintf(exports[f][nexp[f]], 64, "%s", tok[1]);
                    nexp[f]++;
                }
                continue;
            }
            size_t L = strlen(tok[0]);
            if (L && tok[0][L - 1] == ':') {
                tok[0][L - 1] = '\0';
                if (add_sym(tok[0], f, exported(f, tok[0]), err, errlen) < 0) {
                    free(copy);
                    return NULL;
                }
            }
        }
        free(copy);
    }
    for (int f = 0; f < nfiles; f++) {
        for (int i = 0; i < nexp[f]; i++) {
            sym_t *s = NULL;
            for (int k = 0; k < nsyms; k++) {
                if (syms[k].file == f && strcmp(syms[k].name, exports[f][i]) == 0) {
                    s = &syms[k];
                    break;
                }
            }
            if (!s) {
                snprintf(err, (size_t)errlen, "global %s is not defined", exports[f][i]);
                return NULL;
            }
            s->global = 1;
        }
    }

    size_t cap = 1;
    for (int f = 0; f < nfiles; f++) {
        cap += strlen(sources[f]) + 64;
    }
    cap += 4096;
    char *out = calloc(1, cap);
    if (!out) {
        return NULL;
    }
    size_t used = 0;
    for (int f = 0; f < nfiles; f++) {
        char *copy = strdup(sources[f]);
        char *save = NULL;
        for (char *line = strtok_r(copy, "\n", &save); line; line = strtok_r(NULL, "\n", &save)) {
            char raw[256];
            snprintf(raw, sizeof(raw), "%s", line);
            strip_comment(raw);
            char *body = trim(raw);
            if (!*body) {
                continue;
            }
            char buf[256];
            snprintf(buf, sizeof(buf), "%s", body);
            char *tok[8];
            int nt = split_tok(buf, tok, 8);
            if (nt >= 2 && (strcmp(tok[0], ".global") == 0 || strcmp(tok[0], ".globl") == 0)) {
                continue;
            }
            char label[80] = {0};
            int ti = 0;
            size_t L = strlen(tok[0]);
            if (L && tok[0][L - 1] == ':') {
                tok[0][L - 1] = '\0';
                sym_t *s = find_visible(tok[0], f);
                if (!s) {
                    snprintf(err, (size_t)errlen, "missing %s", tok[0]);
                    free(copy);
                    free(out);
                    return NULL;
                }
                rename_sym(s, label, sizeof(label));
                ti = 1;
            }
            if (label[0]) {
                int n = snprintf(out + used, cap - used, "%s:\n", label);
                if (n < 0 || (size_t)n >= cap - used) {
                    snprintf(err, (size_t)errlen, "image too big");
                    free(copy);
                    free(out);
                    return NULL;
                }
                used += (size_t)n;
            }
            if (ti >= nt) {
                continue;
            }
            char rebuilt[256];
            snprintf(rebuilt, sizeof(rebuilt), "%s", tok[ti]);
            int noperands = 0;
            const char *mn = tok[ti];
            int label_pos = -1;
            if (!strcmp(mn, "j") || !strcmp(mn, "jal") || !strcmp(mn, "call")) {
                label_pos = ti + 1;
            } else if (!strcmp(mn, "beq") || !strcmp(mn, "bne")) {
                label_pos = ti + 3;
            }
            for (int t = ti + 1; t < nt; t++) {
                char piece[80];
                sym_t *s = NULL;
                if (is_id(tok[t]) && !is_reg(tok[t])) {
                    s = find_visible(tok[t], f);
                }
                if (s) {
                    rename_sym(s, piece, sizeof(piece));
                } else if (t == label_pos) {
                    snprintf(err, (size_t)errlen, "undefined reference to %s", tok[t]);
                    free(copy);
                    free(out);
                    return NULL;
                } else {
                    snprintf(piece, sizeof(piece), "%s", tok[t]);
                }
                size_t rl = strlen(rebuilt);
                snprintf(rebuilt + rl, sizeof(rebuilt) - rl, noperands ? ", %s" : " %s", piece);
                noperands++;
            }
            size_t add = strlen(rebuilt);
            if (used + add + 2 >= cap) {
                snprintf(err, (size_t)errlen, "image too big");
                free(copy);
                free(out);
                return NULL;
            }
            memcpy(out + used, rebuilt, add);
            used += add;
            out[used++] = '\n';
            out[used] = '\0';
        }
        free(copy);
    }
    return out;
}
