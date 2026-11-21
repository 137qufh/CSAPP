#include "ex_symbols.h"
#include "minitest.h"

static void expect_ok(int kind0, int kind1, int winner) {
    int w = -1;
    MT_EQ_INT(resolve_one(kind0, kind1, &w), 1);
    MT_EQ_INT(w, winner);
}

static void expect_err(int kind0, int kind1) {
    int w = 77;
    MT_EQ_INT(resolve_one(kind0, kind1, &w), 0);
    MT_EQ_INT(w, 77);
}

int main(void) {
    expect_err(2, 2);
    expect_ok(2, 1, 0);
    expect_ok(1, 2, 1);
    expect_ok(1, 1, 0);
    expect_ok(0, 2, 1);
    expect_ok(2, 0, 0);
    expect_ok(0, 1, 1);
    expect_ok(1, 0, 0);
    expect_err(0, 0);
    MT_REPORT();
}
