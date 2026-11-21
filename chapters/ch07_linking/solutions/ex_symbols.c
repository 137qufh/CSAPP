#include "ex_symbols.h"

int resolve_one(int kind0, int kind1, int *winner) {
    if (!winner) {
        return 0;
    }
    if (kind0 < 0 || kind0 > 2 || kind1 < 0 || kind1 > 2) {
        return 0;
    }
    if (kind0 == 0 && kind1 == 0) {
        return 0;
    }
    if (kind0 == 2 && kind1 == 2) {
        return 0;
    }
    if (kind0 == 0) {
        *winner = 1;
        return 1;
    }
    if (kind1 == 0) {
        *winner = 0;
        return 1;
    }
    if (kind0 == 2) {
        *winner = 0;
        return 1;
    }
    if (kind1 == 2) {
        *winner = 1;
        return 1;
    }
    /* Both weak: the first file keeps the symbol. */
    *winner = 0;
    return 1;
}
