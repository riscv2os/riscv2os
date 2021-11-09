#include "memory.h"

struct Chunk {
    char *mem;
    uint32_t begin, size;
};

#define IO_CHUNKS 1
static struct Chunk *chunks, *uart_chunk;
static int nchunk;

struct Chunk *mem_add_chunk(uint32_t begin, uint32_t size, char *mem, uint32_t copy_size) {
    struct Chunk *c = &chunks[nchunk];
    c->begin = begin;
    c->size = size;
    if (c->size == 0) return NULL;
    nchunk++;
    c->mem = calloc(1, c->size); // 不能用 malloc(c->size); 要清零
    if (mem != NULL) memcpy(c->mem, mem, copy_size);
    return c;
}

bool io_init() {
    uart_chunk = mem_add_chunk(UART, UART_CHUNK_SIZE, NULL, 0);
    uart_chunk->mem[LSR] = 0xFF; // 一開始 UART 設定為可輸出
}

bool mem_load_elf(elf_t *e) {
    int chunk_max = e->hdr->e_phnum+IO_CHUNKS;
    chunks = malloc(sizeof(struct Chunk)*chunk_max);
    nchunk = 0;
    for (int i = 0; i < e->hdr->e_phnum; ++i) {
        struct Elf32_Phdr *phdr = &e->phdrs[i];
        if (phdr->p_type != PT_LOAD)
            continue;
        uint32_t begin = phdr->p_vaddr;
        uint32_t size = max(phdr->p_memsz, phdr->p_filesz);
        char *mem = e->rawdata + phdr->p_offset;
        mem_add_chunk(begin, size, mem, phdr->p_filesz);
    }
    io_init();
    return true;
}

struct Chunk *mem_find_chunk(uint32_t addr) {
  for (int i=0; i<nchunk; i++) {
      struct Chunk *c = &chunks[i];
      int end = c->begin + c->size;
      if (addr >= c->begin && addr < end)
        return c;
  }
  ERROR("chunk not found, addr=0x%x!", addr);
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

uint8_t io_write_b(struct Chunk *c, uint32_t addr, uint8_t data) {
    if (addr == UART+THR) { // 當寫入到 UART 時
        // printf("UART: output %c\n", data); // 就模擬在宿主機輸出該字元的行為。
        putc((char)data, stdout); // 就模擬在宿主機輸出該字元的行為。
        c->mem[LSR] |= UART_LSR_EMPTY_MASK; // UART 已經空了，可以再輸出了。
    }
}

uint8_t mem_write_b(uint32_t addr, uint8_t data) {
    struct Chunk *c = mem_find_chunk(addr);
    if (!c || addr >= c->begin+c->size) ERROR("mem_write_b() addr out of range!");
    *(uint8_t*) &c->mem[addr-c->begin] = data;
    io_write_b(c, addr, data);
}

bool mem_free(elf_t *e) {
    for (int i = 0; i < nchunk; i++) {
        free(chunks[i].mem);
    }
    free(chunks);
    return true;
}
