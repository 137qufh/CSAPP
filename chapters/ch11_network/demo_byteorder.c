/* Host order versus network order for a 32-bit value.
 * htonl writes big-endian bytes. On a little-endian machine the numeric
 * value of the swapped word looks different even though the on-wire bytes
 * are 12 34 56 78.
 */

#include <arpa/inet.h>
#include <stdint.h>
#include <stdio.h>

int main(void) {
    uint32_t host = 0x12345678u;
    uint32_t net = htonl(host);
    uint32_t back = ntohl(net);

    printf("htonl(0x%08x)=0x%08x ntohl=0x%08x\n", (unsigned)host, (unsigned)net,
           (unsigned)back);
    printf("note: network order is big-endian; a little-endian host swaps these bytes\n");
    if (back != host) {
        fprintf(stderr, "ntohl did not restore the host value\n");
        return 1;
    }
    printf("order-ok\n");
    return 0;
}
