#include "my86.h"
#include "minitest.h"

#include <string.h>

static int same_arch(const my86_t *a, const my86_t *b) {
    if (memcmp(a->reg, b->reg, sizeof a->reg) != 0) {
        return 0;
    }
    if (strcmp(a->out, b->out) != 0) {
        return 0;
    }
    /* Data region and a bit of low memory (stores). Skip nothing: both images
     * started identical and should still match, including code bytes. */
    return memcmp(a->mem, b->mem, MY86_MEM_SIZE) == 0;
}

static void run_pair(const char *src, uint32_t r1, const char *out, int straight) {
    my86_t *seq = my86_new();
    MT_ASSERT(seq != NULL);
    MT_EQ_INT(my86_assemble(seq, src), 0);
    if (seq->error) {
        fprintf(stderr, "asm: %s\n", seq->err);
    }
    my86_t *pipe = my86_clone(seq);
    MT_ASSERT(pipe != NULL);
    MT_EQ_INT(my86_run_seq(seq, 100000), 0);
    if (seq->error) {
        fprintf(stderr, "seq: %s\n", seq->err);
    }
    MT_EQ_INT(my86_run_pipe(pipe, 100000, 0), 0);
    if (pipe->error) {
        fprintf(stderr, "pipe: %s\n", pipe->err);
    }
    MT_ASSERT(seq->halted);
    MT_ASSERT(pipe->halted);
    MT_EQ_UINT(seq->reg[1], r1);
    MT_EQ_STR(seq->out, out);
    MT_ASSERT(same_arch(seq, pipe));
    MT_EQ_UINT(seq->steps, pipe->steps);
    if (straight) {
        MT_EQ_UINT(pipe->cycles, seq->steps + 4);
    } else {
        MT_ASSERT(pipe->cycles > seq->steps);
    }
    my86_dispose(seq);
    my86_dispose(pipe);
}

int main(void) {
    run_pair("li r1, 1\n"
             "li r2, 2\n"
             "add r3, r1, r2\n"
             "addi r3, r3, 1\n"
             "halt\n",
             1, "", 1);

    run_pair("li r1, 0\n"
             "li r2, 1\n"
             "li r3, 11\n"
             "loop:\n"
             "add r1, r1, r2\n"
             "addi r2, r2, 1\n"
             "beq r2, r3, done\n"
             "j loop\n"
             "done:\n"
             "svc 2\n"
             "halt\n",
             55, "55\n", 0);

    run_pair("li r1, 40\n"
             "jal func\n"
             "addi r1, r1, 1\n"
             "svc 2\n"
             "halt\n"
             "func:\n"
             "addi r1, r1, 2\n"
             "jr lr\n",
             43, "43\n", 0);

    run_pair("li r1, 0x4000\n"
             "li r2, 99\n"
             "sw r2, 0(r1)\n"
             "lw r3, 0(r1)\n"
             "addi r4, r3, 1\n"
             "add r1, r4, r0\n"
             "svc 2\n"
             "halt\n",
             100, "100\n", 0);

    run_pair(".data\n"
             "msg:\n"
             ".byte 72, 105, 10\n"
             ".text\n"
             "li r2, msg\n"
             "lb r1, 0(r2)\n"
             "svc 1\n"
             "lb r1, 1(r2)\n"
             "svc 1\n"
             "lb r1, 2(r2)\n"
             "svc 1\n"
             "halt\n",
             10, "Hi\n", 0);

    /* load-use encoding helper */
    uint32_t lw = my86_pack(MY86_OP_LW, 3, 1, 0, 0);
    uint32_t addi = my86_pack(MY86_OP_ADDI, 4, 3, 0, 1);
    uint32_t other = my86_pack(MY86_OP_ADDI, 4, 1, 0, 1);
    MT_EQ_INT(my86_load_use(lw, addi), 1);
    MT_EQ_INT(my86_load_use(lw, other), 0);
    MT_EQ_INT(my86_load_use(addi, lw), 0);

    MT_REPORT();
}
