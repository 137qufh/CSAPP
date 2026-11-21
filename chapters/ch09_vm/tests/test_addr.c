#include "ex_addr.h"
#include "minitest.h"

int main(void) {
    MT_EQ_UINT(page_offset(0x12345u, 12), 0x345u);
    MT_EQ_UINT(virt_page(0x12345u, 12), 0x12u);
    MT_EQ_UINT(page_offset(0x00abu, 8), 0xabu);
    MT_EQ_UINT(virt_page(0x00abu, 8), 0u);
    MT_EQ_UINT(virt_page(0x0100u, 8), 1u);
    MT_REPORT();
}
