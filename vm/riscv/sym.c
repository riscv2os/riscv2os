#include "sym.h"

c_map_t *sym_load_elf(elf_t *e) {
    c_map_t *symbols = c_map_init(int, char *, c_map_cmp_uint);
    c_map_clear(symbols);
    c_map_insert(symbols, &(int){0}, &(char *){NULL});

    const elf_section_t section = elf_section(e, ".symtab");
    
    struct Elf32_Sym *sym, *symtab = (struct Elf32_Sym *) section.body;
    for (sym=symtab; (char*)sym-(char*)symtab < section.size; sym++) {
        const char *sym_name = e->strtab + sym->st_name;
        switch (ELF_ST_TYPE(sym->st_info)) {
        case STT_NOTYPE:
        case STT_OBJECT:
        case STT_FUNC:
            c_map_insert(symbols, (void *) &(sym->st_value), &sym_name);
        }
    }
    return symbols;
}

const char *sym_find(c_map_t *symbols, uint32_t addr) {
    c_map_iter_t it;
    c_map_find(symbols, &it, &addr);
    return c_map_at_end(symbols, &it) ? NULL : c_map_iter_value(&it, char *);
}

bool sym_free(c_map_t *symbols) {
    c_map_delete(symbols);
}
