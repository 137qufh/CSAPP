#include "ex_parse.h"
#include "minitest.h"

#include <signal.h>
#include <string.h>
#include <unistd.h>

static void on_alarm(int sig) {
    (void)sig;
    _exit(2);
}

static void check_ok(const char *line, const char *want_path) {
    char method[16];
    char path[256];
    int rc = parse_request_line(line, method, sizeof(method), path, sizeof(path));
    MT_EQ_INT(rc, 0);
    if (rc == 0) {
        MT_EQ_STR(method, "GET");
        MT_EQ_STR(path, want_path);
    }
}

static void check_bad(const char *line) {
    char method[16];
    char path[256];
    memset(method, 0, sizeof(method));
    memset(path, 0, sizeof(path));
    MT_EQ_INT(parse_request_line(line, method, sizeof(method), path, sizeof(path)), -1);
}

int main(void) {
    char method[8];
    char path[8];
    struct sigaction sa;

    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = on_alarm;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGALRM, &sa, NULL);
    alarm(5);

    check_ok("GET /index.html HTTP/1.0", "/index.html");
    check_ok("GET /cgi/hello?name=lin HTTP/1.0\r\n", "/cgi/hello?name=lin");
    check_ok("GET / HTTP/1.0", "/");
    check_ok("  GET\t/a/b HTTP/1.1  ", "/a/b");

    check_bad("GET /index.html");
    check_bad("GET /index.html\r\n");
    check_bad("POST /index.html HTTP/1.0");
    check_bad("PUT /index.html HTTP/1.0");
    check_bad("GET  HTTP/1.0");
    check_bad("GET HTTP/1.0");
    check_bad("");
    check_bad("GET /x HTTP");
    check_bad("GET /x HTTP/");
    check_bad("get /x HTTP/1.0");
    check_bad(NULL);

    MT_EQ_INT(parse_request_line("GET /index.html HTTP/1.0", method, 2, path, sizeof(path)),
              -1);
    MT_EQ_INT(parse_request_line("GET /index.html HTTP/1.0", method, sizeof(method), path, 2),
              -1);
    MT_REPORT();
}
