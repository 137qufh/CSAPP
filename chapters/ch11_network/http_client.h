#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H

#include <stddef.h>

/* GET path from host:port with a plain TCP socket.
 * On success returns 0, stores the status code, and mallocs a NUL-terminated
 * body (*body_len excludes that NUL). Caller frees *body_out.
 * path keeps its query string, for example "/cgi/hello?name=lin".
 */
int http_fetch(const char *host, int port, const char *path, int *status_out,
               unsigned char **body_out, size_t *body_len);

#endif
