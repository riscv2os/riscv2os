#pragma once

#include "../lib/lib.h"
#include "../elf/elf.h"

bool mem_load_elf(elf_t *e);
uint32_t mem_read_w(uint32_t addr);
uint16_t mem_read_s(uint32_t addr);
uint8_t mem_read_b(uint32_t addr);
uint32_t mem_write_w(uint32_t addr, uint32_t data);
uint16_t mem_write_s(uint32_t addr, uint16_t data);
uint8_t mem_write_b(uint32_t addr, uint8_t data);