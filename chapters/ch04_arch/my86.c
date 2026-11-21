#include "my86.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

/* ---------- lifecycle ---------- */

my86_t *my86_new(void) {
    my86_t *m = calloc(1, sizeof(*m));
    return m;
}

void my86_dispose(my86_t *m) { free(m); }

void my86_reset(my86_t *m) {
    my86_mem_fn ma = m->mem_access;
    void *ctx = m->mem_ctx;
    my86_svc_fn svc = m->svc;
    memset(m->reg, 0, sizeof(m->reg));
    m->pc = 0;
    m->halted = 0;
    m->error = 0;
    m->err[0] = '\0';
    m->steps = 0;
    m->cycles = 0;
    m->mem_reads = 0;
    m->mem_writes = 0;
    m->out_len = 0;
    m->out[0] = '\0';
    m->pipe_stalls = 0;
    m->pipe_flushes = 0;
    m->pipe_forwards = 0;
    m->mem_access = ma;
    m->mem_ctx = ctx;
    m->svc = svc;
}

my86_t *my86_clone(const my86_t *m) {
    my86_t *c = my86_new();
    if (!c) {
        return NULL;
    }
    memcpy(c, m, sizeof(*c));
    return c;
}

uint32_t my86_pack(int op, int rd, int rs, int rt, int imm16) {
    return ((uint32_t)(op & 0x7f) << 25) | ((uint32_t)(rd & 7) << 22) |
           ((uint32_t)(rs & 7) << 19) | ((uint32_t)(rt & 7) << 16) |
           ((uint32_t)imm16 & 0xffffu);
}

void my86_decode(uint32_t w, my86_dec_t *d) {
    memset(d, 0, sizeof(*d));
    d->op = (int)(w >> 25);
    d->rd = (int)((w >> 22) & 7);
    d->rs = (int)((w >> 19) & 7);
    d->rt = (int)((w >> 16) & 7);
    d->uimm = w & 0xffffu;
    d->imm = (int)(int16_t)(w & 0xffffu);
    switch (d->op) {
    case MY86_OP_ADD: case MY86_OP_SUB: case MY86_OP_AND: case MY86_OP_OR:
    case MY86_OP_XOR: case MY86_OP_SLT: case MY86_OP_SLL: case MY86_OP_SRL:
    case MY86_OP_SRA:
        d->read_rs = d->read_rt = d->write_rd = 1;
        break;
    case MY86_OP_ADDI: case MY86_OP_ANDI: case MY86_OP_ORI: case MY86_OP_XORI:
        d->read_rs = d->write_rd = 1;
        break;
    case MY86_OP_LI: case MY86_OP_LUI:
        d->write_rd = 1;
        break;
    case MY86_OP_LW: case MY86_OP_LB:
        d->read_rs = d->write_rd = d->is_load = 1;
        break;
    case MY86_OP_SW: case MY86_OP_SB:
        d->read_rs = d->read_rd = d->is_store = 1;
        break;
    case MY86_OP_BEQ: case MY86_OP_BNE:
        d->read_rs = d->read_rt = d->is_branch = 1;
        break;
    case MY86_OP_JAL:
        d->write_rd = d->is_branch = 1;
        break;
    case MY86_OP_JR:
        d->read_rs = d->is_branch = 1;
        break;
    case MY86_OP_HALT:
        d->is_halt = 1;
        break;
    case MY86_OP_SVC:
        d->is_svc = 1;
        break;
    default:
        break;
    }
}

static const char *op_name(int op) {
    switch (op) {
    case MY86_OP_NOP: return "nop";
    case MY86_OP_HALT: return "halt";
    case MY86_OP_ADD: return "add";
    case MY86_OP_SUB: return "sub";
    case MY86_OP_AND: return "and";
    case MY86_OP_OR: return "or";
    case MY86_OP_XOR: return "xor";
    case MY86_OP_SLT: return "slt";
    case MY86_OP_SLL: return "sll";
    case MY86_OP_SRL: return "srl";
    case MY86_OP_SRA: return "sra";
    case MY86_OP_ADDI: return "addi";
    case MY86_OP_ANDI: return "andi";
    case MY86_OP_ORI: return "ori";
    case MY86_OP_XORI: return "xori";
    case MY86_OP_LI: return "li";
    case MY86_OP_LUI: return "lui";
    case MY86_OP_LW: return "lw";
    case MY86_OP_SW: return "sw";
    case MY86_OP_LB: return "lb";
    case MY86_OP_SB: return "sb";
    case MY86_OP_BEQ: return "beq";
    case MY86_OP_BNE: return "bne";
    case MY86_OP_JAL: return "jal";
    case MY86_OP_JR: return "jr";
    case MY86_OP_SVC: return "svc";
    default: return "???";
    }
}

void my86_disasm(uint32_t w, char *buf, size_t n) {
    my86_dec_t d;
    my86_decode(w, &d);
    switch (d.op) {
    case MY86_OP_NOP: case MY86_OP_HALT:
        snprintf(buf, n, "%s", op_name(d.op));
        break;
    case MY86_OP_LI: case MY86_OP_LUI:
        snprintf(buf, n, "%s r%d, %d", op_name(d.op), d.rd, d.imm);
        break;
    case MY86_OP_ADDI: case MY86_OP_ANDI: case MY86_OP_ORI: case MY86_OP_XORI:
        snprintf(buf, n, "%s r%d, r%d, %d", op_name(d.op), d.rd, d.rs, d.imm);
        break;
    case MY86_OP_LW: case MY86_OP_LB:
        snprintf(buf, n, "%s r%d, %d(r%d)", op_name(d.op), d.rd, d.imm, d.rs);
        break;
    case MY86_OP_SW: case MY86_OP_SB:
        snprintf(buf, n, "%s r%d, %d(r%d)", op_name(d.op), d.rd, d.imm, d.rs);
        break;
    case MY86_OP_BEQ: case MY86_OP_BNE:
        snprintf(buf, n, "%s r%d, r%d, %+d", op_name(d.op), d.rs, d.rt, d.imm);
        break;
    case MY86_OP_JAL:
        snprintf(buf, n, "jal r%d, %+d", d.rd, d.imm);
        break;
    case MY86_OP_JR:
        snprintf(buf, n, "jr r%d", d.rs);
        break;
    case MY86_OP_SVC:
        snprintf(buf, n, "svc %u", d.uimm);
        break;
    default:
        snprintf(buf, n, "%s r%d, r%d, r%d", op_name(d.op), d.rd, d.rs, d.rt);
        break;
    }
}

int my86_load_use(uint32_t producer, uint32_t consumer) {
    my86_dec_t p, c;
    my86_decode(producer, &p);
    my86_decode(consumer, &c);
    if (!p.is_load || p.rd == 0) {
        return 0;
    }
    if (c.read_rs && c.rs == p.rd) return 1;
    if (c.read_rt && c.rt == p.rd) return 1;
    if (c.read_rd && c.rd == p.rd) return 1;
    if (c.is_svc && p.rd == 1) return 1;
    return 0;
}

/* ---------- memory ---------- */

static void wr32(uint8_t *mem, uint32_t a, uint32_t v) {
    mem[a] = (uint8_t)v;
    mem[a + 1] = (uint8_t)(v >> 8);
    mem[a + 2] = (uint8_t)(v >> 16);
    mem[a + 3] = (uint8_t)(v >> 24);
}

static uint32_t rd32(const uint8_t *mem, uint32_t a) {
    return (uint32_t)mem[a] | ((uint32_t)mem[a + 1] << 8) |
           ((uint32_t)mem[a + 2] << 16) | ((uint32_t)mem[a + 3] << 24);
}

static int default_mem(my86_t *m, uint32_t addr, int size, int is_store, uint32_t *data) {
    if (size != 1 && size != 4) {
        return -1;
    }
    if (addr > MY86_MEM_SIZE || (uint32_t)size > MY86_MEM_SIZE - addr) {
        return -1;
    }
    if (size == 4 && (addr & 3u)) {
        return -1;
    }
    if (is_store) {
        if (size == 4) {
            wr32(m->mem, addr, *data);
        } else {
            m->mem[addr] = (uint8_t)(*data & 0xffu);
        }
    } else if (size == 4) {
        *data = rd32(m->mem, addr);
    } else {
        *data = m->mem[addr];
    }
    return 0;
}

static int access_mem(my86_t *m, uint32_t addr, int size, int is_store, uint32_t *data) {
    if (is_store) {
        m->mem_writes++;
    } else {
        m->mem_reads++;
    }
    if (m->mem_access) {
        void *ctx = m->mem_ctx ? m->mem_ctx : (void *)m;
        return m->mem_access(ctx, addr, size, is_store, data);
    }
    return default_mem(m, addr, size, is_store, data);
}

static void fail(my86_t *m, const char *msg) {
    m->error = 1;
    m->halted = 1;
    snprintf(m->err, sizeof(m->err), "%s", msg);
}

static int do_svc(my86_t *m, unsigned num) {
    if (m->svc) {
        return m->svc(m, (int)num);
    }
    if (num == 0) {
        return 1;
    }
    if (num == 1) {
        if (m->out_len + 1 < (int)sizeof(m->out)) {
            m->out[m->out_len++] = (char)(m->reg[1] & 0xffu);
            m->out[m->out_len] = '\0';
        }
        return 0;
    }
    if (num == 2) {
        char tmp[32];
        int n = snprintf(tmp, sizeof(tmp), "%d\n", (int)m->reg[1]);
        if (n < 0) {
            return 0;
        }
        for (int i = 0; i < n && m->out_len + 1 < (int)sizeof(m->out); i++) {
            m->out[m->out_len++] = tmp[i];
        }
        m->out[m->out_len] = '\0';
        return 0;
    }
    fail(m, "unknown svc");
    return -1;
}

/* ---------- assembler ---------- */

typedef struct {
    char name[64];
    uint32_t addr;
} sym_t;

typedef struct {
    my86_t *m;
    int pass;
    int in_data;
    uint32_t text_pc;
    uint32_t data_pc;
    sym_t syms[512];
    int nsyms;
    int lineno;
} asm_t;

static void aerr(asm_t *a, const char *fmt, ...) {
    va_list ap;
    char buf[120];
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    snprintf(a->m->err, sizeof(a->m->err), "line %d: %s", a->lineno, buf);
    a->m->error = 1;
}

static sym_t *find_sym(asm_t *a, const char *name) {
    for (int i = 0; i < a->nsyms; i++) {
        if (strcmp(a->syms[i].name, name) == 0) {
            return &a->syms[i];
        }
    }
    return NULL;
}

static int add_sym(asm_t *a, const char *name, uint32_t addr) {
    if (strlen(name) >= sizeof(a->syms[0].name)) {
        aerr(a, "label too long");
        return -1;
    }
    if (find_sym(a, name)) {
        aerr(a, "duplicate label %s", name);
        return -1;
    }
    if (a->nsyms >= 512) {
        aerr(a, "too many labels");
        return -1;
    }
    snprintf(a->syms[a->nsyms].name, sizeof(a->syms[0].name), "%s", name);
    a->syms[a->nsyms].addr = addr;
    a->nsyms++;
    return 0;
}

static int parse_num(const char *s, long *out) {
    char *end = NULL;
    if (!s || !*s) {
        return -1;
    }
    long v = strtol(s, &end, 0);
    if (end == s || *end != '\0') {
        return -1;
    }
    *out = v;
    return 0;
}

static int parse_reg(const char *s, int *r) {
    if (!s) {
        return -1;
    }
    if (strcmp(s, "zero") == 0) { *r = 0; return 0; }
    if (strcmp(s, "lr") == 0) { *r = 6; return 0; }
    if (strcmp(s, "sp") == 0) { *r = 7; return 0; }
    if (s[0] == 'r' && s[1] >= '0' && s[1] <= '7' && s[2] == '\0') {
        *r = s[1] - '0';
        return 0;
    }
    return -1;
}

static char *trim(char *s) {
    while (*s && isspace((unsigned char)*s)) s++;
    if (*s == '\0') return s;
    char *e = s + strlen(s);
    while (e > s && isspace((unsigned char)e[-1])) {
        *--e = '\0';
    }
    return s;
}

static void strip_comment(char *s) {
    for (char *p = s; *p; p++) {
        if (*p == '#' || (*p == '/' && p[1] == '/')) {
            *p = '\0';
            return;
        }
    }
}

static int emit_word_at(asm_t *a, uint32_t addr, uint32_t w) {
    if (addr + 4 > MY86_MEM_SIZE) {
        aerr(a, "address out of range");
        return -1;
    }
    if (a->pass == 2) {
        wr32(a->m->mem, addr, w);
    }
    return 0;
}

static int emit_insn(asm_t *a, int op, int rd, int rs, int rt, int imm) {
    if (imm < -32768 || imm > 32767) {
        aerr(a, "immediate %d does not fit in 16 bits", imm);
        return -1;
    }
    uint32_t pc = a->text_pc;
    if (pc + 4 > MY86_DATA_BASE) {
        aerr(a, "text overflow");
        return -1;
    }
    if (emit_word_at(a, pc, my86_pack(op, rd, rs, rt, imm)) < 0) {
        return -1;
    }
    a->text_pc += 4;
    return 0;
}

static int emit_data_byte(asm_t *a, unsigned b) {
    if (a->data_pc >= MY86_MEM_SIZE) {
        aerr(a, "data overflow");
        return -1;
    }
    if (a->pass == 2) {
        a->m->mem[a->data_pc] = (uint8_t)b;
    }
    a->data_pc++;
    return 0;
}

static int resolve_addr(asm_t *a, const char *tok, uint32_t *out) {
    long v;
    if (parse_num(tok, &v) == 0) {
        *out = (uint32_t)v;
        return 0;
    }
    if (a->pass == 1) {
        *out = 0;
        return 0;
    }
    sym_t *s = find_sym(a, tok);
    if (!s) {
        aerr(a, "unknown label %s", tok);
        return -1;
    }
    *out = s->addr;
    return 0;
}

static int split_ops(char *s, char **ops, int maxn) {
    int n = 0;
    while (*s && n < maxn) {
        while (*s && (isspace((unsigned char)*s) || *s == ',')) s++;
        if (!*s) break;
        ops[n++] = s;
        while (*s && *s != ',') s++;
        if (*s == ',') {
            *s++ = '\0';
        }
        trim(ops[n - 1]);
    }
    return n;
}

static int parse_memop(const char *s, int *imm, int *reg) {
    const char *lp = strchr(s, '(');
    const char *rp = strrchr(s, ')');
    if (!lp || !rp || rp < lp) {
        return -1;
    }
    char ib[64], rb[32];
    size_t il = (size_t)(lp - s);
    size_t rl = (size_t)(rp - lp - 1);
    if (il >= sizeof(ib) || rl >= sizeof(rb) || rl == 0) {
        return -1;
    }
    memcpy(ib, s, il);
    ib[il] = '\0';
    memcpy(rb, lp + 1, rl);
    rb[rl] = '\0';
    trim(ib);
    trim(rb);
    long v = 0;
    if (ib[0] == '\0') {
        v = 0;
    } else if (parse_num(ib, &v) < 0) {
        return -1;
    }
    if (v < -32768 || v > 32767) {
        return -1;
    }
    *imm = (int)v;
    return parse_reg(rb, reg);
}

static int asm_one(asm_t *a, char *line) {
    strip_comment(line);
    line = trim(line);
    if (*line == '\0') {
        return 0;
    }
    /* label: */
    char *colon = strchr(line, ':');
    if (colon && colon != line) {
        int ident = 1;
        for (char *p = line; p < colon; p++) {
            if (!(isalnum((unsigned char)*p) || *p == '_' || *p == '.')) {
                ident = 0;
                break;
            }
        }
        if (ident) {
            *colon = '\0';
            if (a->pass == 1) {
                uint32_t addr = a->in_data ? a->data_pc : a->text_pc;
                if (add_sym(a, line, addr) < 0) {
                    return -1;
                }
            }
            line = trim(colon + 1);
            if (*line == '\0') {
                return 0;
            }
        }
    }
    char buf[256];
    snprintf(buf, sizeof(buf), "%s", line);
    char *sp = strchr(buf, ' ');
    char *tab = strchr(buf, '\t');
    if (!sp || (tab && tab < sp)) sp = tab;
    char mnem[32];
    char *rest = NULL;
    if (sp) {
        *sp = '\0';
        rest = trim(sp + 1);
    }
    size_t ml = strlen(buf);
    if (ml >= sizeof(mnem)) {
        aerr(a, "mnemonic too long");
        return -1;
    }
    memcpy(mnem, buf, ml + 1);
    if (strcmp(mnem, ".text") == 0) { a->in_data = 0; return 0; }
    if (strcmp(mnem, ".data") == 0) { a->in_data = 1; return 0; }
    if (strcmp(mnem, ".global") == 0 || strcmp(mnem, ".globl") == 0) {
        return 0;
    }
    if (strcmp(mnem, ".space") == 0 || strcmp(mnem, ".byte") == 0 ||
        strcmp(mnem, ".word") == 0 || strcmp(mnem, ".ascii") == 0 ||
        strcmp(mnem, ".asciiz") == 0) {
        a->in_data = 1;
    }
    if (strcmp(mnem, ".space") == 0) {
        long n;
        if (!rest || parse_num(rest, &n) < 0 || n < 0) {
            aerr(a, "bad .space");
            return -1;
        }
        for (long i = 0; i < n; i++) {
            if (emit_data_byte(a, 0) < 0) return -1;
        }
        return 0;
    }
    if (strcmp(mnem, ".byte") == 0) {
        char *ops[32];
        int n = split_ops(rest ? rest : (char *)"", ops, 32);
        if (n <= 0) { aerr(a, "bad .byte"); return -1; }
        for (int i = 0; i < n; i++) {
            long v;
            if (parse_num(ops[i], &v) < 0) { aerr(a, "bad byte"); return -1; }
            if (emit_data_byte(a, (unsigned)v & 0xffu) < 0) return -1;
        }
        return 0;
    }
    if (strcmp(mnem, ".word") == 0) {
        char *ops[32];
        int n = split_ops(rest ? rest : (char *)"", ops, 32);
        if (n <= 0) { aerr(a, "bad .word"); return -1; }
        for (int i = 0; i < n; i++) {
            uint32_t addr;
            if (resolve_addr(a, ops[i], &addr) < 0) return -1;
            if (a->data_pc + 4 > MY86_MEM_SIZE) { aerr(a, "data overflow"); return -1; }
            if (a->pass == 2) wr32(a->m->mem, a->data_pc, addr);
            a->data_pc += 4;
        }
        return 0;
    }
    if (strcmp(mnem, ".ascii") == 0 || strcmp(mnem, ".asciiz") == 0) {
        if (!rest || rest[0] != '"') { aerr(a, "string expected"); return -1; }
        char *p = rest + 1;
        while (*p && *p != '"') {
            unsigned c = (unsigned char)*p++;
            if (c == '\\') {
                char e = *p++;
                if (e == 'n') c = '\n';
                else if (e == 't') c = '\t';
                else if (e == '0') c = 0;
                else if (e == '\\' || e == '"') c = (unsigned char)e;
                else { aerr(a, "bad escape"); return -1; }
            }
            if (emit_data_byte(a, c) < 0) return -1;
        }
        if (strcmp(mnem, ".asciiz") == 0) {
            if (emit_data_byte(a, 0) < 0) return -1;
        }
        return 0;
    }

    if (a->in_data) {
        aerr(a, "instruction in data section");
        return -1;
    }

    char *ops[8] = {0};
    int nops = split_ops(rest ? rest : (char *)"", ops, 8);

    if (strcmp(mnem, "nop") == 0) return emit_insn(a, MY86_OP_NOP, 0, 0, 0, 0);
    if (strcmp(mnem, "halt") == 0) return emit_insn(a, MY86_OP_HALT, 0, 0, 0, 0);
    if (strcmp(mnem, "ret") == 0) return emit_insn(a, MY86_OP_JR, 0, 6, 0, 0);

    int rd, rs, rt, imm;
    if (strcmp(mnem, "j") == 0) {
        if (nops != 1) { aerr(a, "j target"); return -1; }
        uint32_t tgt;
        if (resolve_addr(a, ops[0], &tgt) < 0) return -1;
        int off = 0;
        if (a->pass == 2) {
            off = (int)tgt - (int)(a->text_pc + 4);
        }
        return emit_insn(a, MY86_OP_BEQ, 0, 0, 0, off);
    }
    if (strcmp(mnem, "jal") == 0 || strcmp(mnem, "call") == 0) {
        if (nops != 1) { aerr(a, "jal target"); return -1; }
        uint32_t tgt;
        if (resolve_addr(a, ops[0], &tgt) < 0) return -1;
        int off = 0;
        if (a->pass == 2) off = (int)tgt - (int)(a->text_pc + 4);
        return emit_insn(a, MY86_OP_JAL, 6, 0, 0, off);
    }
    if (strcmp(mnem, "jr") == 0) {
        if (nops != 1 || parse_reg(ops[0], &rs) < 0) { aerr(a, "jr reg"); return -1; }
        return emit_insn(a, MY86_OP_JR, 0, rs, 0, 0);
    }
    if (strcmp(mnem, "svc") == 0) {
        long v = 0;
        if (nops != 1 || parse_num(ops[0], &v) < 0) { aerr(a, "svc imm"); return -1; }
        return emit_insn(a, MY86_OP_SVC, 0, 0, 0, (int)v);
    }
    if (strcmp(mnem, "mv") == 0) {
        if (nops != 2 || parse_reg(ops[0], &rd) < 0 || parse_reg(ops[1], &rs) < 0) {
            aerr(a, "mv rd, rs"); return -1;
        }
        return emit_insn(a, MY86_OP_ADDI, rd, rs, 0, 0);
    }
    if (strcmp(mnem, "li") == 0 || strcmp(mnem, "lui") == 0) {
        if (nops != 2 || parse_reg(ops[0], &rd) < 0) { aerr(a, "%s rd, imm", mnem); return -1; }
        long v;
        uint32_t addr;
        int immv;
        if (parse_num(ops[1], &v) == 0) {
            immv = (int)v;
        } else {
            if (resolve_addr(a, ops[1], &addr) < 0) return -1;
            immv = (int)addr;
        }
        int op = strcmp(mnem, "li") == 0 ? MY86_OP_LI : MY86_OP_LUI;
        return emit_insn(a, op, rd, 0, 0, immv);
    }

    int arith3 = -1;
    if (strcmp(mnem, "add") == 0) arith3 = MY86_OP_ADD;
    else if (strcmp(mnem, "sub") == 0) arith3 = MY86_OP_SUB;
    else if (strcmp(mnem, "and") == 0) arith3 = MY86_OP_AND;
    else if (strcmp(mnem, "or") == 0) arith3 = MY86_OP_OR;
    else if (strcmp(mnem, "xor") == 0) arith3 = MY86_OP_XOR;
    else if (strcmp(mnem, "slt") == 0) arith3 = MY86_OP_SLT;
    else if (strcmp(mnem, "sll") == 0) arith3 = MY86_OP_SLL;
    else if (strcmp(mnem, "srl") == 0) arith3 = MY86_OP_SRL;
    else if (strcmp(mnem, "sra") == 0) arith3 = MY86_OP_SRA;
    if (arith3 >= 0) {
        if (nops != 3 || parse_reg(ops[0], &rd) < 0 || parse_reg(ops[1], &rs) < 0 ||
            parse_reg(ops[2], &rt) < 0) {
            aerr(a, "%s rd, rs, rt", mnem);
            return -1;
        }
        return emit_insn(a, arith3, rd, rs, rt, 0);
    }

    int immop = -1;
    if (strcmp(mnem, "addi") == 0) immop = MY86_OP_ADDI;
    else if (strcmp(mnem, "andi") == 0) immop = MY86_OP_ANDI;
    else if (strcmp(mnem, "ori") == 0) immop = MY86_OP_ORI;
    else if (strcmp(mnem, "xori") == 0) immop = MY86_OP_XORI;
    if (immop >= 0) {
        long v;
        if (nops != 3 || parse_reg(ops[0], &rd) < 0 || parse_reg(ops[1], &rs) < 0 ||
            parse_num(ops[2], &v) < 0) {
            aerr(a, "%s rd, rs, imm", mnem);
            return -1;
        }
        return emit_insn(a, immop, rd, rs, 0, (int)v);
    }

    if (strcmp(mnem, "beq") == 0 || strcmp(mnem, "bne") == 0) {
        if (nops != 3 || parse_reg(ops[0], &rs) < 0 || parse_reg(ops[1], &rt) < 0) {
            aerr(a, "%s rs, rt, target", mnem);
            return -1;
        }
        uint32_t tgt;
        if (resolve_addr(a, ops[2], &tgt) < 0) return -1;
        int off = 0;
        if (a->pass == 2) off = (int)tgt - (int)(a->text_pc + 4);
        int op = strcmp(mnem, "beq") == 0 ? MY86_OP_BEQ : MY86_OP_BNE;
        return emit_insn(a, op, 0, rs, rt, off);
    }

    if (strcmp(mnem, "lw") == 0 || strcmp(mnem, "sw") == 0 ||
        strcmp(mnem, "lb") == 0 || strcmp(mnem, "sb") == 0) {
        if (nops != 2 || parse_reg(ops[0], &rd) < 0 || parse_memop(ops[1], &imm, &rs) < 0) {
            aerr(a, "%s rd, imm(rs)", mnem);
            return -1;
        }
        int op = MY86_OP_LW;
        if (strcmp(mnem, "sw") == 0) op = MY86_OP_SW;
        else if (strcmp(mnem, "lb") == 0) op = MY86_OP_LB;
        else if (strcmp(mnem, "sb") == 0) op = MY86_OP_SB;
        return emit_insn(a, op, rd, rs, 0, imm);
    }

    aerr(a, "unknown mnemonic %s", mnem);
    return -1;
}

int my86_assemble(my86_t *m, const char *src) {
    my86_mem_fn ma = m->mem_access;
    void *ctx = m->mem_ctx;
    my86_svc_fn svc = m->svc;
    memset(m->mem, 0, sizeof(m->mem));
    my86_reset(m);
    m->mem_access = ma;
    m->mem_ctx = ctx;
    m->svc = svc;

    char *copy = strdup(src ? src : "");
    if (!copy) {
        fail(m, "oom");
        return -1;
    }
    char *lines[4000];
    int nlines = 0;
    char *p = copy;
    while (nlines < 4000) {
        lines[nlines++] = p;
        char *nl = strchr(p, '\n');
        if (!nl) break;
        *nl = '\0';
        p = nl + 1;
    }
    asm_t a;
    memset(&a, 0, sizeof(a));
    a.m = m;
    a.data_pc = MY86_DATA_BASE;
    for (int pass = 1; pass <= 2; pass++) {
        a.pass = pass;
        a.in_data = 0;
        a.text_pc = 0;
        a.data_pc = MY86_DATA_BASE;
        if (pass == 2) {
            /* symbols already collected */
        }
        for (int i = 0; i < nlines; i++) {
            a.lineno = i + 1;
            char buf[256];
            snprintf(buf, sizeof(buf), "%s", lines[i]);
            /* drop CR */
            size_t L = strlen(buf);
            if (L && buf[L - 1] == '\r') buf[L - 1] = '\0';
            if (asm_one(&a, buf) < 0) {
                free(copy);
                return -1;
            }
        }
    }
    free(copy);
    m->pc = 0;
    return 0;
}

/* ---------- sequential core ---------- */

static int exec_alu(my86_t *m, const my86_dec_t *d, uint32_t *res, int *write) {
    uint32_t rs = m->reg[d->rs];
    uint32_t rt = m->reg[d->rt];
    *write = d->write_rd && d->rd != 0;
    switch (d->op) {
    case MY86_OP_NOP: *write = 0; return 0;
    case MY86_OP_HALT: *write = 0; return 1;
    case MY86_OP_ADD: *res = rs + rt; return 0;
    case MY86_OP_SUB: *res = rs - rt; return 0;
    case MY86_OP_AND: *res = rs & rt; return 0;
    case MY86_OP_OR: *res = rs | rt; return 0;
    case MY86_OP_XOR: *res = rs ^ rt; return 0;
    case MY86_OP_SLT: *res = ((int32_t)rs < (int32_t)rt) ? 1u : 0u; return 0;
    case MY86_OP_SLL: *res = rs << (rt & 31u); return 0;
    case MY86_OP_SRL: *res = rs >> (rt & 31u); return 0;
    case MY86_OP_SRA: *res = (uint32_t)((int32_t)rs >> (rt & 31u)); return 0;
    case MY86_OP_ADDI: *res = rs + (uint32_t)d->imm; return 0;
    case MY86_OP_ANDI: *res = rs & d->uimm; return 0;
    case MY86_OP_ORI: *res = rs | d->uimm; return 0;
    case MY86_OP_XORI: *res = rs ^ d->uimm; return 0;
    case MY86_OP_LI: *res = (uint32_t)d->imm; return 0;
    case MY86_OP_LUI: *res = d->uimm << 16; return 0;
    default: *write = 0; return 0;
    }
}

int my86_run_seq(my86_t *m, uint64_t max_steps) {
    while (!m->halted && m->steps < max_steps) {
        uint32_t w = 0;
        if (access_mem(m, m->pc, 4, 0, &w) < 0) {
            fail(m, "fetch fault");
            return -1;
        }
        my86_dec_t d;
        my86_decode(w, &d);
        uint32_t pc = m->pc;
        uint32_t next = pc + 4;
        uint32_t res = 0;
        int write = 0;
        int stop = exec_alu(m, &d, &res, &write);
        if (d.is_load) {
            uint32_t addr = m->reg[d.rs] + (uint32_t)d.imm;
            uint32_t data = 0;
            int sz = (d.op == MY86_OP_LB) ? 1 : 4;
            if (access_mem(m, addr, sz, 0, &data) < 0) {
                fail(m, "load fault");
                return -1;
            }
            if (d.op == MY86_OP_LB) {
                res = (uint32_t)(int32_t)(int8_t)(data & 0xffu);
            } else {
                res = data;
            }
            write = d.rd != 0;
        } else if (d.is_store) {
            uint32_t addr = m->reg[d.rs] + (uint32_t)d.imm;
            uint32_t data = m->reg[d.rd];
            int sz = (d.op == MY86_OP_SB) ? 1 : 4;
            if (access_mem(m, addr, sz, 1, &data) < 0) {
                fail(m, "store fault");
                return -1;
            }
        } else if (d.op == MY86_OP_BEQ || d.op == MY86_OP_BNE) {
            int eq = m->reg[d.rs] == m->reg[d.rt];
            int take = (d.op == MY86_OP_BEQ) ? eq : !eq;
            if (take) next = (uint32_t)((int)pc + 4 + d.imm);
        } else if (d.op == MY86_OP_JAL) {
            res = pc + 4;
            write = d.rd != 0;
            next = (uint32_t)((int)pc + 4 + d.imm);
        } else if (d.op == MY86_OP_JR) {
            next = m->reg[d.rs];
        } else if (d.op == MY86_OP_SVC) {
            int s = do_svc(m, d.uimm);
            if (s < 0) return -1;
            if (s > 0) stop = 1;
        } else if (d.op != MY86_OP_NOP && d.op != MY86_OP_HALT && !d.write_rd &&
                   d.op != MY86_OP_ADD && stop == 0 && d.op > MY86_OP_SVC) {
            fail(m, "bad opcode");
            return -1;
        }
        if (d.op > MY86_OP_SVC) {
            fail(m, "bad opcode");
            return -1;
        }
        if (write && d.rd != 0) {
            m->reg[d.rd] = res;
        }
        m->reg[0] = 0;
        m->steps++;
        if (stop || d.is_halt) {
            m->halted = 1;
            m->pc = next;
            return 0;
        }
        m->pc = next;
    }
    if (!m->halted) {
        fail(m, "step limit");
        return -1;
    }
    return 0;
}

/* ---------- 5-stage pipeline ---------- */

typedef struct {
    int bubble;
    int is_halt, is_load, is_store, is_branch, is_svc;
    int taken;
    uint32_t pc, insn, target;
    int op, rd, rs, rt, imm;
    unsigned uimm;
    int read_rs, read_rt, read_rd, write_rd;
    uint32_t val_rs, val_rt, val_rd;
    uint32_t alu;
    uint32_t memv;
    char text[48];
} preg_t;

static preg_t bubble(void) {
    preg_t p;
    memset(&p, 0, sizeof(p));
    p.bubble = 1;
    snprintf(p.text, sizeof(p.text), "bubble");
    return p;
}

static int preg_uses(const preg_t *d, int reg) {
    if (d->bubble || reg == 0) return 0;
    if (d->read_rs && d->rs == reg) return 1;
    if (d->read_rt && d->rt == reg) return 1;
    if (d->read_rd && d->rd == reg) return 1;
    if (d->is_svc && reg == 1) return 1;
    return 0;
}

int my86_run_pipe(my86_t *m, uint64_t max_cycles, int verbose) {
    preg_t D = bubble(), E = bubble(), M = bubble(), W = bubble();
    uint32_t fpc = 0;
    int stop_fetch = 0;

    while (m->cycles < max_cycles && !m->error) {
        m->cycles++;
        /* Commit writeback first so the regfile matches an edge-triggered WB. */
        if (!W.bubble && W.is_halt) {
            m->steps++;
            m->halted = 1;
            m->reg[0] = 0;
            return 0;
        }
        if (!W.bubble) {
            if (W.write_rd && W.rd != 0) {
                m->reg[W.rd] = W.is_load ? W.memv : W.alu;
            }
            m->steps++;
            m->reg[0] = 0;
            if (W.is_svc) {
                uint32_t saved = m->reg[1];
                m->reg[1] = W.val_rs; /* svc arg captured as val_rs */
                int s = do_svc(m, W.uimm);
                m->reg[1] = saved;
                if (s < 0) {
                    return -1;
                }
                if (s > 0) {
                    m->halted = 1;
                    return 0;
                }
            }
        }

        /* Memory stage. */
        preg_t nW = M;
        if (!M.bubble && M.is_load) {
            uint32_t data = 0;
            int sz = (M.op == MY86_OP_LB) ? 1 : 4;
            if (access_mem(m, M.alu, sz, 0, &data) < 0) {
                fail(m, "pipe load fault");
                return -1;
            }
            if (M.op == MY86_OP_LB) {
                nW.memv = (uint32_t)(int32_t)(int8_t)(data & 0xffu);
            } else {
                nW.memv = data;
            }
        } else if (!M.bubble && M.is_store) {
            uint32_t data = M.val_rd;
            int sz = (M.op == MY86_OP_SB) ? 1 : 4;
            if (access_mem(m, M.alu, sz, 1, &data) < 0) {
                fail(m, "pipe store fault");
                return -1;
            }
        }

        /* Forward into Execute. */
        uint32_t frs = E.val_rs, frt = E.val_rt, frd = E.val_rd;
        if (!E.bubble) {
            int fwd_reg[3] = {E.rs, E.rt, E.rd};
            uint32_t *slot[3] = {&frs, &frt, &frd};
            int use[3] = {E.read_rs, E.read_rt, E.read_rd};
            if (E.is_svc) {
                /* r1 is the syscall argument, stashed in val_rs. */
                fwd_reg[0] = 1;
                use[0] = 1;
                frs = m->reg[1];
            }
            for (int i = 0; i < 3; i++) {
                if (!use[i] || fwd_reg[i] == 0) continue;
                int reg = fwd_reg[i];
                if (!M.bubble && M.write_rd && M.rd == reg && !M.is_load) {
                    *slot[i] = M.alu;
                    m->pipe_forwards++;
                } else if (!W.bubble && W.write_rd && W.rd == reg) {
                    *slot[i] = W.is_load ? W.memv : W.alu;
                    m->pipe_forwards++;
                }
            }
        }

        preg_t nM = E;
        if (!E.bubble) {
            nM.val_rs = frs;
            nM.val_rt = frt;
            nM.val_rd = frd;
            nM.taken = 0;
            uint32_t rs = frs, rt = frt;
            switch (E.op) {
            case MY86_OP_ADD: nM.alu = rs + rt; break;
            case MY86_OP_SUB: nM.alu = rs - rt; break;
            case MY86_OP_AND: nM.alu = rs & rt; break;
            case MY86_OP_OR: nM.alu = rs | rt; break;
            case MY86_OP_XOR: nM.alu = rs ^ rt; break;
            case MY86_OP_SLT: nM.alu = ((int32_t)rs < (int32_t)rt) ? 1u : 0u; break;
            case MY86_OP_SLL: nM.alu = rs << (rt & 31u); break;
            case MY86_OP_SRL: nM.alu = rs >> (rt & 31u); break;
            case MY86_OP_SRA: nM.alu = (uint32_t)((int32_t)rs >> (rt & 31u)); break;
            case MY86_OP_ADDI: nM.alu = rs + (uint32_t)E.imm; break;
            case MY86_OP_ANDI: nM.alu = rs & E.uimm; break;
            case MY86_OP_ORI: nM.alu = rs | E.uimm; break;
            case MY86_OP_XORI: nM.alu = rs ^ E.uimm; break;
            case MY86_OP_LI: nM.alu = (uint32_t)E.imm; break;
            case MY86_OP_LUI: nM.alu = E.uimm << 16; break;
            case MY86_OP_LW: case MY86_OP_LB:
            case MY86_OP_SW: case MY86_OP_SB:
                nM.alu = rs + (uint32_t)E.imm;
                break;
            case MY86_OP_BEQ: case MY86_OP_BNE: {
                int eq = rs == rt;
                nM.taken = (E.op == MY86_OP_BEQ) ? eq : !eq;
                nM.target = (uint32_t)((int)E.pc + 4 + E.imm);
                break;
            }
            case MY86_OP_JAL:
                nM.alu = E.pc + 4;
                nM.taken = 1;
                nM.target = (uint32_t)((int)E.pc + 4 + E.imm);
                break;
            case MY86_OP_JR:
                nM.taken = 1;
                nM.target = rs;
                break;
            case MY86_OP_SVC:
                nM.val_rs = frs; /* r1 */
                nM.uimm = E.uimm;
                break;
            default:
                break;
            }
        }

        int stall = (!E.bubble && E.is_load && E.rd != 0 && preg_uses(&D, E.rd));
        int flush = (!E.bubble && E.is_branch && nM.taken);
        int halt_here = (!E.bubble && E.is_halt);

        preg_t nE = bubble();
        preg_t nD = bubble();
        uint32_t nfpc = fpc;

        if (stall) {
            m->pipe_stalls++;
            nE = bubble();
            nD = D;
            nfpc = fpc;
            /* The load must still flow E->M; nM already holds it.
               Inserting a bubble in E next cycle breaks the load-use. */
        } else if (flush || halt_here) {
            if (flush) {
                m->pipe_flushes++;
                nfpc = nM.target;
            } else {
                stop_fetch = 1;
                nfpc = fpc;
            }
            nE = bubble();
            nD = bubble();
        } else {
            /* D -> E, reading the regfile after this cycle's WB. */
            if (!D.bubble) {
                nE = D;
                nE.bubble = 0;
                nE.val_rs = m->reg[D.rs];
                nE.val_rt = m->reg[D.rt];
                nE.val_rd = m->reg[D.rd];
                if (D.is_svc) {
                    nE.val_rs = m->reg[1];
                }
            }
            if (!stop_fetch) {
                uint32_t w = 0;
                if (access_mem(m, fpc, 4, 0, &w) < 0) {
                    fail(m, "pipe fetch fault");
                    return -1;
                }
                my86_dec_t dec;
                my86_decode(w, &dec);
                nD.bubble = 0;
                nD.pc = fpc;
                nD.insn = w;
                nD.op = dec.op;
                nD.rd = dec.rd;
                nD.rs = dec.rs;
                nD.rt = dec.rt;
                nD.imm = dec.imm;
                nD.uimm = dec.uimm;
                nD.read_rs = dec.read_rs;
                nD.read_rt = dec.read_rt;
                nD.read_rd = dec.read_rd;
                nD.write_rd = dec.write_rd;
                nD.is_load = dec.is_load;
                nD.is_store = dec.is_store;
                nD.is_branch = dec.is_branch;
                nD.is_halt = dec.is_halt;
                nD.is_svc = dec.is_svc;
                my86_disasm(w, nD.text, sizeof(nD.text));
                nfpc = fpc + 4;
            } else {
                nD = bubble();
                nfpc = fpc;
            }
        }

        if (verbose) {
            printf("c%-4llu F:%08x D:%-16s E:%-16s M:%-16s W:%-16s%s\n",
                   (unsigned long long)m->cycles, fpc,
                   D.bubble ? "bubble" : D.text,
                   E.bubble ? "bubble" : E.text,
                   M.bubble ? "bubble" : M.text,
                   W.bubble ? "bubble" : W.text,
                   stall ? "  stall" : (flush ? "  flush" : ""));
        }

        W = nW;
        M = nM;
        E = nE;
        D = nD;
        fpc = nfpc;
        m->pc = fpc;

        int alive = !D.bubble || !E.bubble || !M.bubble || !W.bubble;
        if (!alive && stop_fetch) {
            break;
        }
    }
    if (!m->halted) {
        fail(m, "pipe cycle limit");
        return -1;
    }
    return 0;
}
