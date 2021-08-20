#include "memory.h"

#define MB      1024*1024
#define NMEMORY 128*MB      // 最大記憶體範圍不可超過 128M

static char memory[NMEMORY];
static uint32_t mlow, mhigh;

bool mem_load_elf(elf_t *e) {
    // rv_set_pc(rv, e->hdr->e_entry); /* set the entry point */
    uint32_t low, high; // 找出最小與最大的記憶體位址，記憶體在此範圍內才是合法的 (這樣也可以少宣告些)
    /* loop over all of the program headers */
    for (int p = 0; p < e->hdr->e_phnum; ++p) {
        /* find next program header */
        uint32_t offset = e->hdr->e_phoff + (p * e->hdr->e_phentsize);
        const struct Elf32_Phdr *phdr = (const struct Elf32_Phdr *) (e->rawdata + offset);
        /* check this section should be loaded */
        if (phdr->p_type != PT_LOAD)
            continue;
        
        if (phdr->p_vaddr < low) low = phdr->p_vaddr;

        int pend = phdr->p_vaddr + max(phdr->p_memsz, phdr->p_filesz);
        if (pend > high) high = pend;
    }
    mlow = low;
    mhigh = high;
    return true;
}

uint32_t mem_read_w(uint32_t addr) {
    if (addr+3 >= mhigh) error();
    return *(uint32_t*) &memory[addr-mlow];
}

uint16_t mem_read_s(uint32_t addr) {
    if (addr+1 >= mhigh) error();
    return *(uint16_t*) &memory[addr-mlow];
}

uint8_t mem_read_b(uint32_t addr) {
    if (addr >= mhigh) error();
    return *(uint8_t*) &memory[addr-mlow];
}

uint32_t mem_write_w(uint32_t addr, uint32_t data) {
    if (addr+3 >= mhigh) error();
    *(uint32_t*) &memory[addr-mlow] = data;
}

uint16_t mem_write_s(uint32_t addr, uint16_t data) {
    if (addr+1 >= mhigh) error();
    *(uint16_t*) &memory[addr-mlow] = data;
}

uint8_t mem_write_b(uint32_t addr, uint8_t data) {
    if (addr >= mhigh) error();
    *(uint8_t*) &memory[addr-mlow] = data;
}
