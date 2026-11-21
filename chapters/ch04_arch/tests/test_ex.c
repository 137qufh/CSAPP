#include "ex_adder.h"
#include "ex_hazard.h"
#include "minitest.h"

int main(void) {
    uint32_t lw = (0x20u << 25) | (3u << 22) | (1u << 19);
    uint32_t use = (0x10u << 25) | (4u << 22) | (3u << 19) | 1u;
    uint32_t other = (0x10u << 25) | (4u << 22) | (2u << 19) | 1u;
    uint32_t sw = (0x21u << 25) | (3u << 22) | (1u << 19);
    MT_EQ_INT(is_load_use(lw, use), 1);
    MT_EQ_INT(is_load_use(lw, other), 0);
    MT_EQ_INT(is_load_use(use, lw), 0);
    MT_EQ_INT(is_load_use(lw, sw), 1);

    MT_EQ_INT(gate_xor(0, 0), 0);
    MT_EQ_INT(gate_xor(1, 0), 1);
    MT_EQ_INT(gate_xor(1, 1), 0);
    MT_EQ_INT(gate_mux(0, 1, 0), 1);
    MT_EQ_INT(gate_mux(1, 1, 0), 0);
    for (int a = 0; a < 16; a++) {
        for (int b = 0; b < 16; b++) {
            MT_EQ_INT(ripple_add4(a, b), (a + b) & 0xF);
        }
    }
    MT_REPORT();
}
