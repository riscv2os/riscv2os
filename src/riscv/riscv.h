#pragma once

#include "decoder.h"

#define NREG 32
#define NCSR 0x1000

extern uint32_t inst;
extern uint32_t op, rd, rs1, rs2, funct3, funct7;
extern uint32_t i_imm, b_imm, u_imm, j_imm, s_imm, csr_imm, csr_id;
extern uint32_t x[NREG];
extern uint32_t csr[NCSR];

bool rv_init_csr();
bool rv_init();
bool rv_decode(uint32_t inst);
bool rv_dasm_block(char *block, int addr, int size);
bool rv_dasm_inst(uint32_t inst, char *dasm, uint32_t pc);
