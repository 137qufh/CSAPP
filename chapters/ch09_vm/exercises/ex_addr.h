#ifndef EX_ADDR_H
#define EX_ADDR_H

/* page_shift is log2(page size). va is a 32-bit virtual address. */
unsigned virt_page(unsigned va, unsigned page_shift);
unsigned page_offset(unsigned va, unsigned page_shift);

#endif
