#include "riscv.h"

uint32_t op, rd, rs1, rs2, funct3, funct7;
uint32_t i_imm, b_imm, u_imm, j_imm, s_imm, csr_imm, csr;

bool rv_decode(uint32_t inst) {
    op = (inst & INST_6_2) >> 2;
    rd = dec_rd(inst);
    funct3 = dec_funct3(inst);
    rs1 = csr_imm = dec_rs1(inst);
    rs2 = dec_rs2(inst);
    funct7 = dec_funct7(inst);
    i_imm = dec_itype_imm(inst);
    b_imm = dec_btype_imm(inst);
    u_imm = dec_utype_imm(inst);
    j_imm = dec_jtype_imm(inst);
    s_imm = dec_stype_imm(inst);
    csr = dec_csr(inst);
}
