#include <stdio.h>
#include "../elf/elf.h"
#include "riscv.h"

bool rv_dasm_code(char *code, int addr, int size) {
    char line[256];
    uint32_t *endp = (uint32_t *) (code + size);
    for (uint32_t *ir = (uint32_t *) code; ir<endp; ir++) {
        if (*ir == 0) {
          while (*ir == 0) {
            ir++;
          }
          ir--;
          printf("...\n");
          continue;
        }
        uint32_t pc = addr+(char*)ir-code;
        rv_decode(*ir);
        rv_dasm(*ir, line, pc);
        printf("%09x %09x %s\n", pc, *ir, line);
    }
}

elf_t elf;

int main(int argc, char *argv[]) {
    char *fname=argv[1];
    if (!elf_load(&elf, fname)) {
        fprintf(stderr, "ELF file '%s' load fail \n", fname);
        return 1; 
    }
    elf_section_t s = elf_section(&elf, ".text");
    printf(".text: body=%p offset=0x%x size=%u\n", s.body, s.addr, s.size);
    csr_init();
    rv_dasm_code(s.body, s.addr, s.size);
}
