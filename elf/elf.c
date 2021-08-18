#include "elf.h"

char *elf_sh_name(elf_t *e, int offset) {
  return &e->shstrtab[offset];
}

char *elf_sym_name(elf_t *e, int offset) {
  return &e->strtab[offset];
}

int elf_find_section(elf_t *e, char *name) {
  for (int si=0; si< e->hdr->e_shnum; si++) {
    char *tname = elf_sh_name(e, e->shdrs[si].sh_name);
    if (strcmp(name, tname)==0)
      return si;
  }
  return -1;
}

elf_section_t elf_get_section(elf_t *e, int si) {
  elf_section_t s;
  if (si > e->hdr->e_shnum || si < 0) {
    s.body = NULL;
  } else {
    struct Elf32_Shdr *h = &e->shdrs[si];
    s.name = elf_sh_name(e, e->shdrs[si].sh_name);
    s.body = &e->rawdata[h->sh_offset];
    s.addr = e->shdrs[si].sh_addr;
    s.size = e->shdrs[si].sh_size;
  }
  return s;
}

elf_section_t elf_section(elf_t *e, char *name) {
  int si = elf_find_section(e, name);
  printf("elf_find_section: name=%s si=%d\n", name, si);
  return elf_get_section(e, si);
}

bool elf_valid(elf_t *e) {
  // 檢查 ELF 魔數
  char e_indent[] = { '\x7f', 'E', 'L', 'F'};
  if (memcmp(e->hdr->e_ident, e_indent, 4)) return false;
  // 檢查是否為 32bit 機器碼
  if (e->hdr->e_ident[EI_CLASS] != ELFCLASS32)
    return false;
  return true;
}

void elf_dump_hdr(elf_t *e) {
  struct Elf32_Hdr *h = e->hdr;
  printf("ELF:\n  type:%d \t(1=ELF32, 2=ELF64)\n  machine:%d \t(40=ARM, 50=IA_64, 243=RISCV)\n  version:%d\n  flags:%d\n  entry:0x%x\n",
                h->e_type, h->e_machine, h->e_version, h->e_flags, h->e_entry);
}

void elf_dump_shdrs(elf_t *e) {
  printf("\nSection Headers:(type: 1=PROGBITS, 2=SYMTAB, 3=STRTAB)\n");
  printf("  [Nr] Name                 Type            Address          Off    Size   ES Flg Lk Inf Al\n");
  for (int i=0; i < e->hdr->e_shnum; i++) {
    struct Elf32_Shdr *h = &e->shdrs[i];
    char *name = elf_sh_name(e, h->sh_name);
    printf("       %-20s %-15x %016lx %06lx %06lx %02lx %3lx %2d %3d %2ld\n", name, h->sh_type, h->sh_addr, h->sh_offset, h->sh_size, h->sh_entsize, h->sh_flags, h->sh_link, h->sh_info, h->sh_addralign);
  }
}

void elf_dump_phdrs(elf_t *e) {
  printf("\nProgram Headers: (type: 1=LOAD, 2=DYNAMIC, ... SHLIB=5 ...)\n");
  printf("  Type           Offset   VirtAddr           PhysAddr           FileSiz  MemSiz   Flg Align\n");

  for (int i=0; i < e->hdr->e_phnum; i++) {
    struct Elf32_Phdr *h = &e->phdrs[i];
    printf("  %-14x 0x%06lx 0x%016lx 0x%016lx 0x%06lx 0x%06lx %3x 0x%lx\n", h->p_type, h->p_offset, h->p_vaddr, h->p_paddr, h->p_filesz, h->p_memsz, h->p_flags, h->p_align);
  }
}

void elf_dump_header(elf_t *e) {
  elf_dump_hdr(e);
  elf_dump_shdrs(e);
  elf_dump_phdrs(e);
}

void stab_dump(char *b, int size) {
  for (int i=0; i<size; i++) {
    char ch = b[i];
    if (ch == '\0') ch = ' ';
    printf("%c", ch);
  }
}

void hex_dump(uint8_t *b, int size) {
  for (int i=0; i<size; i++) {
    printf("%02x ", b[i]);
    if (i >= 100) {
      printf("...");
      break;
    }
  }
}
/*
struct Elf32_Sym {
    Elf32_Word st_name;
    Elf32_Addr st_value;
    Elf32_Word st_size;
    uint8_t st_info;
    uint8_t st_other;
    Elf32_Half st_shndx;
};
*/
// ref: http://labmaster.mi.infn.it/Laboratorio2/CompilerCD/clang/l1/ELF.html
void elf_dump_symtab(elf_t *e, char *b, int size) {
  struct Elf32_Sym *p, *symtab = (struct Elf32_Sym *) b;
  for (p=symtab; (char*) p-b < size; p++) {
    // char *name = (p->st_info==0)?elf_sym_name(e, p->st_name):elf_sh_name(e, p->st_name);
    int bind = p->info >> 4;  // STB_LOCAL:0  STB_GLOBAL:1   STB_WEAK:2   STB_LOPROC:13  STB_HIPROC:15
    int type = p->info & 0xf; // STT_NOTYPE:0  STT_OBJECT:1 STT_FUNC:2 STT_SECTION:3 STT_FILE:4 STT_LOPROC:13  STT_HIPROC:15
    char *name = elf_sym_name(e, p->st_name);
    printf("name:%s value:0x%x size:%d info:%d other:%d shndx:0x%x\n", name, p->st_value, p->st_size, p->st_info, p->st_other, p->st_shndx);
    // shndx 是段代號
  }
}

void elf_dump_section(elf_t *e, int si) {
  elf_section_t s = elf_get_section(e, si);
  printf("%s\n", s.name);
  if (strcmp(s.name, ".strtab")==0 || strcmp(s.name, ".shstrtab")==0 || strcmp(s.name, ".riscv.attributes")==0) {
    stab_dump(s.body, s.size);
  } else if (strcmp(s.name, ".symtab")==0) {
    elf_dump_symtab(e, s.body, s.size);
  } else {
    hex_dump(s.body, s.size);
  }
  printf("\n");
}

void elf_dump_body(elf_t *e) {
  for (int si=0; si< e->hdr->e_shnum; si++) {
    elf_dump_section(e, si);
  }
}

void elf_dump(elf_t *e) {
  elf_dump_header(e);
  elf_dump_body(e);
}

bool elf_load(elf_t *e, const char *path) {
    FILE *file = fopen(path, "rb");

    if (!file) return false;

    e->file = file;
    
    e->rawsize = fread(e->rawdata, 1, sizeof(e->rawdata), file);
    e->hdr = (struct Elf32_Hdr *)e->rawdata;

    if (!elf_valid(e)) return false;

    e->shdrs = (struct Elf32_Shdr*) &e->rawdata[e->hdr->e_shoff];
    e->phdrs = (struct Elf32_Phdr*) &e->rawdata[e->hdr->e_phoff];

    struct Elf32_Shdr *sh = &e->shdrs[e->hdr->e_shstrndx];

    e->shstrtab = &e->rawdata[sh->sh_offset];

    e->strtab = elf_section(e, ".strtab").body;
    return true;
}
