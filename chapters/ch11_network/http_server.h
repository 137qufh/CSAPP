#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

/* Serve HTTP/1.0 on an already listening socket.
 * One request per accepted connection, then the socket is closed.
 * Returns only when accept fails (the caller closed the fd, or a signal
 * stopped the process).
 */
int http_serve_fd(int listen_fd);

#endif
