#include <stdio.h>

/* TODO: concurrent HTTP/1.0 proxy.
 * Listen on 127.0.0.1 port 0, print "port N" and flush.
 * Accept absolute-form GET http://host:port/path HTTP/1.0
 * Forward it, cache 200 responses in an LRU, and serve later hits locally.
 */
int main(void) {
    fprintf(stderr, "proxy not implemented\n");
    return 1;
}
