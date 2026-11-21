#include "ex_parse.h"

int parse_request_line(const char *line, char *method, size_t mlen, char *path,
                       size_t plen) {
    (void)line;
    (void)method;
    (void)mlen;
    (void)path;
    (void)plen;
    /* TODO: accept only "GET /path HTTP/1.x", reject an empty path. */
    return -1;
}
