#ifndef SHELLCRAFT_EVAL_H
#define SHELLCRAFT_EVAL_H

void shell_init(void);
/* Execute one line. quit/exit may terminate the process. */
void eval_line(const char *line);

#endif
