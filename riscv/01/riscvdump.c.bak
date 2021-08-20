#include <stdio.h>
#include "../elf/elf.h"
#include "riscv.h"

elf_t elf;

int main(int argc, char *argv[]) {
    /* open the ELF file from the file system */
    char *fname=argv[1];
    if (!elf_load(&elf, argv[1])) {
        fprintf(stderr, "ELF file '%s' load fail \n", fname);
        return 1; 
    }
    elf_section_t s = elf_section(&elf, ".text");
    printf("s.body=%p offset=0x%x size=%u\n", s.body, s.addr, s.size);
    rv_decode(s.body, s.addr, s.size);
}
