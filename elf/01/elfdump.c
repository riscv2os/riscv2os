#include <stdio.h>
#include "elf.h"

elf_t elf;

int main(int argc, char *argv[]) {
    /* open the ELF file from the file system */
    char *fname=argv[1];
    if (!elf_load(&elf, argv[1])) {
        fprintf(stderr, "ELF file '%s' load fail \n", fname);
        return 1; 
    }
}
