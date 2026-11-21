#include "ex_hazard.h"

static int op_of(uint32_t w) { return (int)(w >> 25); }
static int rd_of(uint32_t w) { return (int)((w >> 22) & 7); }
static int rs_of(uint32_t w) { return (int)((w >> 19) & 7); }
static int rt_of(uint32_t w) { return (int)((w >> 16) & 7); }

static int reads(uint32_t w, int reg) {
    if (reg == 0) {
        return 0;
    }
    int op = op_of(w);
    int rs = rs_of(w);
    int rt = rt_of(w);
    int rd = rd_of(w);
    switch (op) {
    case 0x02: case 0x03: case 0x04: case 0x05: case 0x06:
    case 0x07: case 0x08: case 0x09: case 0x0A:
        return reg == rs || reg == rt;
    case 0x10: case 0x11: case 0x12: case 0x13:
    case 0x20: case 0x22:
        return reg == rs;
    case 0x21: case 0x23:
        return reg == rs || reg == rd;
    case 0x30: case 0x31:
        return reg == rs || reg == rt;
    case 0x33:
        return reg == rs;
    case 0x34:
        return reg == 1;
    default:
        return 0;
    }
}

int is_load_use(uint32_t producer, uint32_t consumer) {
    int op = op_of(producer);
    if (op != 0x20 && op != 0x22) {
        return 0;
    }
    int rd = rd_of(producer);
    if (rd == 0) {
        return 0;
    }
    return reads(consumer, rd);
}
