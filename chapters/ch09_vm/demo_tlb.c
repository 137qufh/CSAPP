#include <stdio.h>

/* Tiny 2-level page table (256-byte pages) plus a 4-entry fully associative TLB.
 * VPN[7:4] selects the page directory, VPN[3:0] the second-level entry.
 */

#define TLB 4

typedef struct {
    int valid;
    unsigned vpn, ppn, stamp;
} tlb_e;

static tlb_e tlb[TLB];
static unsigned stamp;
static unsigned hits, misses;
static int present[16][16];
static unsigned ppn[16][16];

static int translate(unsigned va, unsigned *pa) {
    unsigned vpn = va >> 8;
    unsigned off = va & 0xffu;
    unsigned l1 = vpn >> 4;
    unsigned l2 = vpn & 0xfu;
    if (vpn >= 256 || !present[l1][l2]) {
        return -1;
    }
    for (int i = 0; i < TLB; i++) {
        if (tlb[i].valid && tlb[i].vpn == vpn) {
            tlb[i].stamp = ++stamp;
            hits++;
            *pa = (tlb[i].ppn << 8) | off;
            return 0;
        }
    }
    misses++;
    int slot = 0;
    for (int i = 0; i < TLB; i++) {
        if (!tlb[i].valid || tlb[i].stamp < tlb[slot].stamp) {
            slot = i;
        }
    }
    tlb[slot].valid = 1;
    tlb[slot].vpn = vpn;
    tlb[slot].ppn = ppn[l1][l2];
    tlb[slot].stamp = ++stamp;
    *pa = (ppn[l1][l2] << 8) | off;
    return 0;
}

int main(void) {
    for (unsigned vpn = 0; vpn < 32; vpn++) {
        present[vpn >> 4][vpn & 0xf] = 1;
        ppn[vpn >> 4][vpn & 0xf] = vpn;
    }
    unsigned pa = 0;
    if (translate(0x1234, &pa) != 0 || pa != 0x1234) {
        return 1;
    }
    if (translate(0x1234, &pa) != 0 || hits != 1) {
        return 1;
    }
    /* Fill the TLB with other pages, then touch 0x12xx again: it may miss. */
    for (unsigned i = 0; i < 8; i++) {
        if (translate((i + 1) << 8, &pa) != 0) {
            return 1;
        }
    }
    printf("tlb hits=%u misses=%u pa=%u\n", hits, misses, pa);
    if (misses < 1) {
        return 1;
    }
    printf("tlb-ok\n");
    return 0;
}
