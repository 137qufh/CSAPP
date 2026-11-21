#ifndef SHELLCRAFT_PARSE_H
#define SHELLCRAFT_PARSE_H

#define SC_MAXARGS 32
#define SC_MAXCMDS 8

typedef struct {
    char *argv[SC_MAXARGS];
    int argc;
    char *infile;
    char *outfile;
    int append;
} sc_cmd_t;

typedef struct {
    sc_cmd_t cmds[SC_MAXCMDS];
    int ncmds;
    int background;
    char text[1024];
} sc_line_t;

/* Parse one command line. Returns 0 on success. Caller frees with sc_line_free. */
int sc_parse(const char *line, sc_line_t *out);
void sc_line_free(sc_line_t *line);

#endif
