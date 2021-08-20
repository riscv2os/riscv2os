#pragma once

#include "decoder.h"

#define NREG 32
#define NCSR 0x1000

bool rv_init_csr();
bool rv_init();
bool rv_decode(uint32_t inst);
bool rv_dasm_block(char *block, int addr, int size);
bool rv_dasm_inst(uint32_t inst, char *dasm, uint32_t pc);