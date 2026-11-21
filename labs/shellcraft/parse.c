#include "parse.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int read_word(const char **ps, char *buf, size_t n) {
    const char *s = *ps;
    size_t i = 0;
    if (*s == '"') {
        s++;
        while (*s && *s != '"') {
            if (i + 1 >= n) {
                return -1;
            }
            buf[i++] = *s++;
        }
        if (*s == '"') {
            s++;
        }
    } else {
        while (*s && !isspace((unsigned char)*s) && !strchr("|<>&", *s)) {
            if (i + 1 >= n) {
                return -1;
            }
            buf[i++] = *s++;
        }
    }
    buf[i] = '\0';
    *ps = s;
    return i == 0 ? -1 : 0;
}

int sc_parse(const char *line, sc_line_t *out) {
    memset(out, 0, sizeof(*out));
    if (!line) {
        return 0;
    }
    snprintf(out->text, sizeof(out->text), "%s", line);
    const char *s = line;
    int cmd = 0;
    while (*s) {
        while (*s && isspace((unsigned char)*s)) {
            s++;
        }
        if (*s == '\0' || *s == '#') {
            break;
        }
        if (*s == '|') {
            cmd++;
            if (cmd >= SC_MAXCMDS) {
                return -1;
            }
            s++;
            continue;
        }
        if (*s == '&') {
            out->background = 1;
            s++;
            continue;
        }
        if (*s == '<' || *s == '>') {
            char kind = *s++;
            int append = 0;
            if (kind == '>' && *s == '>') {
                append = 1;
                s++;
            }
            while (*s && isspace((unsigned char)*s)) {
                s++;
            }
            char word[256];
            if (read_word(&s, word, sizeof(word)) < 0) {
                return -1;
            }
            if (out->ncmds < cmd + 1) {
                out->ncmds = cmd + 1;
            }
            if (kind == '<') {
                out->cmds[cmd].infile = strdup(word);
            } else {
                out->cmds[cmd].outfile = strdup(word);
                out->cmds[cmd].append = append;
            }
            continue;
        }
        char word[256];
        if (read_word(&s, word, sizeof(word)) < 0) {
            return -1;
        }
        if (cmd >= SC_MAXCMDS) {
            return -1;
        }
        if (out->ncmds < cmd + 1) {
            out->ncmds = cmd + 1;
        }
        sc_cmd_t *c = &out->cmds[cmd];
        if (c->argc >= SC_MAXARGS - 1) {
            return -1;
        }
        c->argv[c->argc++] = strdup(word);
        c->argv[c->argc] = NULL;
    }
    return 0;
}

void sc_line_free(sc_line_t *line) {
    if (!line) {
        return;
    }
    for (int i = 0; i < line->ncmds; i++) {
        sc_cmd_t *c = &line->cmds[i];
        for (int a = 0; a < c->argc; a++) {
            free(c->argv[a]);
            c->argv[a] = NULL;
        }
        free(c->infile);
        free(c->outfile);
        c->infile = c->outfile = NULL;
        c->argc = 0;
    }
    line->ncmds = 0;
}
