#include "ex_addr.h"

unsigned virt_page(unsigned va, unsigned page_shift) { return va >> page_shift; }

unsigned page_offset(unsigned va, unsigned page_shift) {
    return va & ((1u << page_shift) - 1u);
}
