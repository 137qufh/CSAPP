#ifndef CAPSTONE_LINK_H
#define CAPSTONE_LINK_H

/* Concatenate MY86 modules. File-local labels are prefixed so they do not
 * collide. Names listed with `.global` stay as written and must be defined
 * once. Returns a newly allocated source string, or NULL on error.
 */
char *link_modules(char **sources, int nfiles, char *err, int errlen);

#endif
