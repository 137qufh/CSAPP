#ifndef EX_HAZARD_H
#define EX_HAZARD_H

#include <stdint.h>

/* MY86 word: opcode[31:25] rd[24:22] rs[21:19] rt[18:16] imm[15:0].
 * Loads are opcodes 0x20 (lw) and 0x22 (lb). A load-use hazard exists when
 * the load's rd is not r0 and the very next instruction reads that register
 * (as rs, rt, or, for a store, as the data register rd). svc (0x34) reads r1.
 */
int is_load_use(uint32_t producer, uint32_t consumer);

#endif
