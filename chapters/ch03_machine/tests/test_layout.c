#include "ex_layout.h"
#include "minitest.h"

int main(void) {
    MT_EQ_INT(align_up(0, 4), 0);
    MT_EQ_INT(align_up(1, 4), 4);
    MT_EQ_INT(align_up(4, 4), 4);
    MT_EQ_INT(align_up(5, 4), 8);
    MT_EQ_INT(align_up(5, 8), 8);
    MT_EQ_INT(align_up(8, 8), 8);
    MT_EQ_INT(align_up(9, 8), 16);
    MT_EQ_INT(align_up(17, 8), 24);
    MT_EQ_INT(align_up(32, 16), 32);
    MT_EQ_INT(align_up(33, 16), 48);
    MT_EQ_INT(align_up(3, 1), 3);
    MT_EQ_INT(align_up(7, 1), 7);
    MT_EQ_INT(align_up(9, 2), 10);
    MT_EQ_INT(align_up(2, 2), 2);
    MT_EQ_INT(align_up(100, 32), 128);
    MT_EQ_INT(align_up(6, 4), 8);

    MT_EQ_INT(off_a(), 0);
    MT_EQ_INT(off_b(), 4);
    MT_EQ_INT(off_c(), 8);
    MT_EQ_INT(span(), 12);
    MT_REPORT();
}
