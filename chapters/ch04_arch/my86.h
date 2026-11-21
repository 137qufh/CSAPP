#ifndef MY86_H
#define MY86_H

/* MY86: an original 32-bit teaching ISA. Not compatible with any course's Y86.
 *
 * Instruction word, little-endian:
 *   [31:25] opcode   [24:22] rd   [21:19] rs   [18:16] rt   [15:0] immediate
 * r0 is hardwired to zero. r6 is the link register (lr). r7 is the stack
 * pointer by software convention (sp). Branches and jal are PC-relative:
 *   target = pc + 4 + sign_extend(imm16).
 * See README.md for the full opcode list.
 */

#include <stddef.h>
#include <stdint.h>

#define MY86_MEM_SIZE 65536u
#define MY86_DATA_BASE 0x4000u
#define MY86_OP_NOP 0x00
#define MY86_OP_HALT 0x01
#define MY86_OP_ADD 0x02
#define MY86_OP_SUB 0x03
#define MY86_OP_AND 0x04
#define MY86_OP_OR 0x05
#define MY86_OP_XOR 0x06
#define MY86_OP_SLT 0x07
#define MY86_OP_SLL 0x08
#define MY86_OP_SRL 0x09
#define MY86_OP_SRA 0x0A
#define MY86_OP_ADDI 0x10
#define MY86_OP_ANDI 0x11
#define MY86_OP_ORI 0x12
#define MY86_OP_XORI 0x13
#define MY86_OP_LI 0x14
#define MY86_OP_LUI 0x15
#define MY86_OP_LW 0x20
#define MY86_OP_SW 0x21
#define MY86_OP_LB 0x22
#define MY86_OP_SB 0x23
#define MY86_OP_BEQ 0x30
#define MY86_OP_BNE 0x31
#define MY86_OP_JAL 0x32
#define MY86_OP_JR 0x33
#define MY86_OP_SVC 0x34

typedef struct my86 my86_t;

/* size is 1 or 4. is_store nonzero means write *data. Return 0 or -1. */
typedef int (*my86_mem_fn)(void *ctx, uint32_t addr, int size, int is_store,
                           uint32_t *data);

/* Optional syscall hook. num is the svc immediate. Return nonzero to halt.
 * When NULL, the built-in hook prints (svc 1 = char in r1, svc 2 = decimal).
 */
typedef int (*my86_svc_fn)(my86_t *m, int num);

struct my86 {
    uint32_t reg[8];
    uint32_t pc;
    uint8_t mem[MY86_MEM_SIZE];
    int halted;
    int error;
    char err[160];
    uint64_t steps;     /* instructions retired (sequential or pipeline) */
    uint64_t cycles;    /* pipeline cycles; sequential leaves this 0 */
    uint64_t mem_reads;
    uint64_t mem_writes;
    char out[4096];
    int out_len;
    my86_mem_fn mem_access; /* NULL -> internal mem[] */
    void *mem_ctx;
    my86_svc_fn svc;
    int pipe_stalls;
    int pipe_flushes;
    int pipe_forwards;
};

typedef struct my86_dec {
    int op, rd, rs, rt;
    int imm;          /* sign-extended */
    unsigned uimm;    /* zero-extended */
    int write_rd;
    int read_rs, read_rt, read_rd;
    int is_load, is_store, is_branch, is_halt, is_svc;
} my86_dec_t;

my86_t *my86_new(void);
void my86_dispose(my86_t *m);
void my86_reset(my86_t *m);
my86_t *my86_clone(const my86_t *m);

uint32_t my86_pack(int op, int rd, int rs, int rt, int imm16);
void my86_decode(uint32_t word, my86_dec_t *d);
void my86_disasm(uint32_t word, char *buf, size_t n);

/* Assemble source into m->mem and reset architectural state. */
int my86_assemble(my86_t *m, const char *src);

int my86_run_seq(my86_t *m, uint64_t max_steps);
/* verbose nonzero prints one line per cycle to stdout. */
int my86_run_pipe(my86_t *m, uint64_t max_cycles, int verbose);

int my86_load_use(uint32_t producer, uint32_t consumer);

#endif
