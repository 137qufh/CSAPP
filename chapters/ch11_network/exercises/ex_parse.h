#ifndef EX_PARSE_H
#define EX_PARSE_H

#include <stddef.h>

/* Parse an HTTP/1.0 request line: "GET /path HTTP/1.0".
 * Return 0 on success and copy the method and the whole path (query string
 * included) into the output buffers. Return -1 on a malformed line.
 * Only GET is accepted.
 */
int parse_request_line(const char *line, char *method, size_t mlen, char *path,
                       size_t plen);

#endif
