#include "memory.h"

struct Chunk {
    char *mem;
    uint32_t begin, size;
};

static struct Chunk *chunks;
static int    nchunk;
#define IO_CHUNKS 1

bool mem_set_chunk(int i, uint32_t begin, uint32_t size, char *mem) {
    struct Chunk *c = &chunks[i];
    c->begin = begin;
    c->size = size;
    // printf("chunk:begin=0x%x, size=0x%x, end=0x%x\n", c->begin, c->size, c->end);
    if (c->size > 0) {
        c->mem = malloc(c->size);
        memcpy(c->mem, mem, c->size);
    } else {
        c->mem = NULL;
    }
}

bool mem_load_elf(elf_t *e) {
    // printf("mem_load_elf() begin\n");
    nchunk = e->hdr->e_phnum+IO_CHUNKS;
    chunks = malloc(sizeof(struct Chunk)*nchunk);
    // printf("e_phnum=%d\n", e->hdr->e_phnum);
    for (int i = 0; i < e->hdr->e_phnum; ++i) {
        struct Elf32_Phdr *phdr = &e->phdrs[i];
        // printf("phdr[%d]\n", i);
        if (phdr->p_type != PT_LOAD)
            continue;
        uint32_t begin = phdr->p_vaddr;
        uint32_t size = max(phdr->p_memsz, phdr->p_filesz);
        char *mem = e->rawdata + phdr->p_offset;
        mem_set_chunk(i, begin, size, mem);
        // printf("phdr: end\n");
    }
    char *uart_mem = malloc(UART_SIZE);
    mem_set_chunk(nchunk-1, UART, UART_SIZE, uart_mem);
    return true;
}

struct Chunk *mem_find_chunk(uint32_t addr) {
  for (int i=0; i<nchunk; i++) {
      struct Chunk *c = &chunks[i];
      int end = c->begin + c->size;
      if (addr >= c->begin && addr < end)
        return c;
  }
  ERROR("chunk not found!");
  return NULL;
}

uint32_t mem_read_w(uint32_t addr) {
    struct Chunk *c = mem_find_chunk(addr);
    if (!c || addr+3 >= c->begin+c->size) ERROR("mem_read_w() addr out of range!");
    return *(uint32_t*) &c->mem[addr-c->begin];
}

uint16_t mem_read_s(uint32_t addr) {
    struct Chunk *c = mem_find_chunk(addr);
    if (!c || addr+1 >= c->begin+c->size) ERROR("mem_read_s() addr out of range!");
    return *(uint16_t*) &c->mem[addr-c->begin];
}

uint8_t mem_read_b(uint32_t addr) {
    struct Chunk *c = mem_find_chunk(addr);
    if (!c || addr >= c->begin+c->size) ERROR("mem_read_b() addr out of range!");
    return *(uint8_t*) &c->mem[addr-c->begin];
}

uint32_t mem_write_w(uint32_t addr, uint32_t data) {
    struct Chunk *c = mem_find_chunk(addr);
    if (!c || addr+3 >= c->begin+c->size) ERROR("mem_write_w() addr out of range!");
    *(uint32_t*) &c->mem[addr-c->begin] = data;
}

uint16_t mem_write_s(uint32_t addr, uint16_t data) {
    struct Chunk *c = mem_find_chunk(addr);
    if (!c || addr+1 >= c->begin+c->size) ERROR("mem_write_s() addr out of range!");
    *(uint16_t*) &c->mem[addr-c->begin] = data;
}

uint8_t mem_write_b(uint32_t addr, uint8_t data) {
    struct Chunk *c = mem_find_chunk(addr);
    if (!c || addr >= c->begin+c->size) ERROR("mem_write_b() addr out of range!");
    *(uint8_t*) &c->mem[addr-c->begin] = data;
    if (addr == UART+THR) {
        printf("UART: put data(%c)", (char)data);
        // putc((char)data);
        c->mem[LSR] &=  ~UART_LSR_EMPTY_MASK;
    }
}

bool mem_free(elf_t *e) {
    for (int i = 0; i < nchunk; i++) {
        free(chunks[i].mem);
    }
    free(chunks);
    return true;
}
