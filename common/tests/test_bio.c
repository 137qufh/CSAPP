#include "minitest.h"
#include "sc_bio.h"

#include <fcntl.h>
#include <unistd.h>

static void test_roundtrip(void) {
    char tmpl[] = "/tmp/scbioXXXXXX";
    int fd = mkstemp(tmpl);
    MT_ASSERT(fd >= 0);
    const char *msg = "alpha\nbeta\ngamma";
    MT_EQ_INT(sc_writen(fd, msg, strlen(msg)), (long)strlen(msg));

    int rd = open(tmpl, O_RDONLY);
    MT_ASSERT(rd >= 0);
    char buf[32];
    MT_EQ_INT(sc_readn(rd, buf, strlen(msg)), (long)strlen(msg));
    buf[strlen(msg)] = '\0';
    MT_EQ_STR(buf, msg);
    close(rd);

    rd = open(tmpl, O_RDONLY);
    sc_bio_t b;
    sc_bio_init(&b, rd);
    MT_ASSERT(sc_bio_readline(&b, buf, sizeof(buf)) > 0);
    MT_EQ_STR(buf, "alpha\n");
    MT_ASSERT(sc_bio_readline(&b, buf, sizeof(buf)) > 0);
    MT_EQ_STR(buf, "beta\n");
    MT_ASSERT(sc_bio_readline(&b, buf, sizeof(buf)) > 0);
    MT_EQ_STR(buf, "gamma");
    MT_EQ_INT(sc_bio_readline(&b, buf, sizeof(buf)), 0);
    close(rd);
    close(fd);
    unlink(tmpl);
}

static void test_short_buffer_line(void) {
    int fds[2];
    MT_ASSERT(pipe(fds) == 0);
    const char *msg = "hello-world\n";
    MT_ASSERT(sc_writen(fds[1], msg, strlen(msg)) == (ssize_t)strlen(msg));
    close(fds[1]);
    sc_bio_t b;
    sc_bio_init(&b, fds[0]);
    char small[6];
    ssize_t n = sc_bio_readline(&b, small, sizeof(small));
    MT_EQ_INT(n, 5);
    MT_EQ_STR(small, "hello");
    n = sc_bio_readline(&b, small, sizeof(small));
    MT_ASSERT(n > 0);
    close(fds[0]);
}

int main(void) {
    test_roundtrip();
    test_short_buffer_line();
    MT_REPORT();
}
