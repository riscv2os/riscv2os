#pragma once

#include "../lib/elf.h"
#include "../lib/c_map.h"

c_map_t *sym_load_elf(elf_t *e);
const char *sym_find(c_map_t *symbols, uint32_t addr);
bool sym_free(c_map_t *symbols);
