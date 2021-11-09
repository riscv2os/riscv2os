#include "lib/elf.h"
#include "riscv/riscv.h"

elf_t elf;

int main(int argc, char *argv[]) {
    char *fname=argv[1];
    if (!elf_load(&elf, fname)) {
        fprintf(stderr, "ELF file '%s' load fail \n", fname);
        return 1; 
    }
    elf_section_t s = elf_section(&elf, ".text");
    printf(".text: body=%p offset=0x%x size=%u\n", s.body, s.addr, s.size);
    rv_init();
    rv_dasm_block(s.body, s.addr, s.size);
}
