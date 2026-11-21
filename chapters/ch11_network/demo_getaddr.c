/* Numeric getaddrinfo for the loopback address. No DNS, no remote host. */

#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>

int main(void) {
    struct addrinfo hints;
    struct addrinfo *res = NULL;
    int rc;
    int family;
    const char *name = "other";

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_NUMERICHOST | AI_NUMERICSERV;

    rc = getaddrinfo("127.0.0.1", "80", &hints, &res);
    if (rc != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rc));
        return 0;
    }
    family = res->ai_family;
    if (family == AF_INET) {
        name = "AF_INET";
    } else if (family == AF_INET6) {
        name = "AF_INET6";
    }
    printf("family=%s (%d)\n", name, family);
    printf("note: AI_NUMERICHOST parses the address locally and does not query DNS\n");
    freeaddrinfo(res);
    if (family != AF_INET && family != AF_INET6) {
        fprintf(stderr, "unexpected family\n");
        return 1;
    }
    printf("addr-ok\n");
    return 0;
}
