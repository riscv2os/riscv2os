#pragma once

#include "../lib/lib.h"
#include "../lib/elf.h"

#define UART       0x10000000
#define THR        0x00    // THR:transmitter holding register
#define LSR        0x05    // LSR:line status register
#define UART_CHUNK_SIZE  0x10
#define UART_LSR_EMPTY_MASK 0x40 

bool mem_load_elf(elf_t *e);
uint32_t mem_read_w(uint32_t addr);
uint16_t mem_read_s(uint32_t addr);
uint8_t mem_read_b(uint32_t addr);
uint32_t mem_write_w(uint32_t addr, uint32_t data);
uint16_t mem_write_s(uint32_t addr, uint16_t data);
uint8_t mem_write_b(uint32_t addr, uint8_t data);
bool mem_free(elf_t *e);
