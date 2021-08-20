#pragma once

#include <stdio.h>
#include <stdlib.h>
#include "decoder.h"

bool cpu_init();
bool cpu_decode(uint32_t inst);
bool cpu_dasm_inst(uint32_t inst, char *dasm, uint32_t pc);
