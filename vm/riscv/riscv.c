#include "riscv.h"

bool rv_init() {
    rv_init_csr();
}

bool rv_dasm_block(char *block, int addr, int size) {
    char line[256];
    uint32_t *endp = (uint32_t *) (block + size);
    for (uint32_t *ir = (uint32_t *) block; ir<endp; ir++) {
        if (*ir == 0) {
          while (*ir == 0) {
            ir++;
          }
          ir--;
          printf("...\n");
          continue;
        }
        uint32_t pc = addr+(char*)ir-block;
        rv_decode(*ir);
        rv_dasm_inst(*ir, line, pc);
        printf("%09x %09x %s\n", pc, *ir, line);
    }
}
