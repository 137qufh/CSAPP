#ifndef SC_ERROR_H
#define SC_ERROR_H

#include <stddef.h>
#include <sys/socket.h>
#include <sys/types.h>

/* Original POSIX helpers for the Systems Craft labs.
 * Names are sc_* on purpose so they are not a drop-in of any textbook package.
 */

void sc_app_error(const char *fmt, ...);
void sc_unix_error(const char *fmt, ...);

void *sc_malloc(size_t size);
void *sc_calloc(size_t n, size_t size);
void *sc_realloc(void *ptr, size_t size);

int sc_open(const char *path, int flags, mode_t mode);
void sc_close(int fd);
ssize_t sc_read(int fd, void *buf, size_t n);
ssize_t sc_write(int fd, const void *buf, size_t n);
void sc_dup2(int oldfd, int newfd);

pid_t sc_fork(void);
pid_t sc_waitpid(pid_t pid, int *status, int options);
void sc_execve(const char *path, char *const argv[], char *const envp[]);
void sc_kill(pid_t pid, int sig);

int sc_socket(int domain, int type, int protocol);
void sc_bind(int fd, const struct sockaddr *addr, socklen_t len);
void sc_listen(int fd, int backlog);
int sc_accept(int fd, struct sockaddr *addr, socklen_t *len);
void sc_connect(int fd, const struct sockaddr *addr, socklen_t len);

/* Pick an ephemeral port on 127.0.0.1 and listen. Returns fd, writes port. */
int sc_listen_local(int *port_out);

#endif
