#pragma once

#include "cpu.h"

bool rv_init();
bool rv_decode(uint32_t inst);
bool rv_dasm_block(char *block, int addr, int size);
