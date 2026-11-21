#include "ex_parse.h"

#include <string.h>

static int is_sp(char c) {
    return c == ' ' || c == '\t';
}

static int copy_token(const char **pp, char *dst, size_t cap) {
    const char *p = *pp;
    size_t i = 0;
    if (cap == 0) {
        return -1;
    }
    while (*p && !is_sp(*p) && *p != '\r' && *p != '\n') {
        if (i + 1 >= cap) {
            return -1;
        }
        dst[i++] = *p++;
    }
    dst[i] = '\0';
    if (i == 0) {
        return -1;
    }
    *pp = p;
    return 0;
}

int parse_request_line(const char *line, char *method, size_t mlen, char *path,
                       size_t plen) {
    const char *p;
    char version[32];
    if (!line || !method || !path || mlen == 0 || plen == 0) {
        return -1;
    }
    p = line;
    while (is_sp(*p)) {
        p++;
    }
    if (copy_token(&p, method, mlen) < 0) {
        return -1;
    }
    if (!is_sp(*p)) {
        return -1;
    }
    while (is_sp(*p)) {
        p++;
    }
    if (copy_token(&p, path, plen) < 0 || path[0] != '/') {
        return -1;
    }
    if (!is_sp(*p)) {
        return -1;
    }
    while (is_sp(*p)) {
        p++;
    }
    if (copy_token(&p, version, sizeof(version)) < 0) {
        return -1;
    }
    if (strncmp(version, "HTTP/", 5) != 0 || version[5] == '\0') {
        return -1;
    }
    while (is_sp(*p) || *p == '\r' || *p == '\n') {
        p++;
    }
    if (*p != '\0') {
        return -1;
    }
    if (strcmp(method, "GET") != 0) {
        return -1;
    }
    return 0;
}
