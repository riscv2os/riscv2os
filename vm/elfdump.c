#include <stdio.h>
#include "lib/elf.h"

elf_t elf;

int main(int argc, char *argv[]) {
    char *fname=argv[1];
    if (!elf_load(&elf, argv[1])) {
        fprintf(stderr, "ELF file '%s' load fail \n", fname);
        return 1; 
    }
    elf_dump(&elf);
}
