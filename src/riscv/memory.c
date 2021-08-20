#include "memory.h"

struct Chunk {
    char *mem;
    uint32_t begin, size, end;
};

static struct Chunk *chunks;
static int    nchunk;

bool mem_load_elf(elf_t *e) {
    nchunk = e->hdr->e_phnum;
    chunks = malloc(sizeof(struct Chunk)*nchunk);
    for (int i = 0; i < e->hdr->e_phnum; ++i) {
        struct Elf32_Phdr *phdr = &e->phdrs[i];
        if (phdr->p_type != PT_LOAD)
            continue;
        struct Chunk *c = &chunks[nchunk++];
        c->begin = phdr->p_vaddr;
        c->size = max(phdr->p_memsz, phdr->p_filesz);
        c->end = c->begin+c->size;
        c->mem = malloc(c->size);
        memcpy(c->mem, e->rawdata + phdr->p_offset, c->size);
    }
    return true;
}

struct Chunk *mem_find_chunk(uint32_t addr) {
  for (int i=0; i<nchunk; i++) {
      struct Chunk *c = &chunks[i];
      if (addr >= c->begin && addr < c->end)
        return c;
  }
  return NULL;
}

uint32_t mem_read_w(uint32_t addr) {
    struct Chunk *c = mem_find_chunk(addr);
    if (!c || addr+3 >= c->end) error();
    return *(uint32_t*) &c->mem[addr-c->begin];
}

uint16_t mem_read_s(uint32_t addr) {
    struct Chunk *c = mem_find_chunk(addr);
    if (!c || addr+1 >= c->end) error();
    return *(uint16_t*) &c->mem[addr-c->begin];
}

uint8_t mem_read_b(uint32_t addr) {
    struct Chunk *c = mem_find_chunk(addr);
    if (!c || addr >= c->end) error();
    return *(uint8_t*) &c->mem[addr-c->begin];
}

uint32_t mem_write_w(uint32_t addr, uint32_t data) {
    struct Chunk *c = mem_find_chunk(addr);
    if (!c || addr+3 >= c->end) error();
    *(uint32_t*) &c->mem[addr-c->begin] = data;
}

uint16_t mem_write_s(uint32_t addr, uint16_t data) {
    struct Chunk *c = mem_find_chunk(addr);
    if (!c || addr+1 >= c->end) error();
    *(uint16_t*) &c->mem[addr-c->begin] = data;
}

uint8_t mem_write_b(uint32_t addr, uint8_t data) {
    struct Chunk *c = mem_find_chunk(addr);
    if (!c || addr >= c->end) error();
    *(uint8_t*) &c->mem[addr-c->begin] = data;
}

bool mem_free(elf_t *e) {
    for (int i = 0; i < nchunk; i++) {
        free(chunks[i].mem);
    }
    free(chunks);
    return true;
}
