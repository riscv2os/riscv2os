#include "elf.h"

char *elf_str(elf_t *e, int offset) {
  return &e->stab[offset];
}

char *elf_section(elf_t *e, int si) {
  struct Elf32_Shdr *h = &e->shdrs[si];
  return &e->rawdata[h->sh_offset];
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

void elf_print_hdr(elf_t *e) {
  struct Elf32_Hdr *h = e->hdr;
  printf("ELF:\n  type:%d \t(1=ELF32, 2=ELF64)\n  machine:%d \t(40=ARM, 50=IA_64, 243=RISCV)\n  version:%d\n  flags:%d\n  entry:0x%x\n",
                h->e_type, h->e_machine, h->e_version, h->e_flags, h->e_entry);
}

void elf_print_shdrs(elf_t *e) {
  printf("\nSection Headers:(type: 1=PROGBITS, 2=SYMTAB, 3=STRTAB)\n");
  printf("  [Nr] Name                 Type            Address          Off    Size   ES Flg Lk Inf Al\n");
  for (int i=0; i < e->hdr->e_shnum; i++) {
    struct Elf32_Shdr *h = &e->shdrs[i];
    char *name = elf_str(e, h->sh_name);
    printf("       %-20s %-15x %016lx %06lx %06lx %02lx %3lx %2d %3d %2ld\n", name, h->sh_type, h->sh_addr, h->sh_offset, h->sh_size, h->sh_entsize, h->sh_flags, h->sh_link, h->sh_info, h->sh_addralign);
  }
}

void elf_print_phdrs(elf_t *e) {
  printf("\nProgram Headers: (type: 1=LOAD, 2=DYNAMIC, ... SHLIB=5 ...)\n");
  printf("  Type           Offset   VirtAddr           PhysAddr           FileSiz  MemSiz   Flg Align\n");

  for (int i=0; i < e->hdr->e_phnum; i++) {
    struct Elf32_Phdr *h = &e->phdrs[i];
    printf("  %-14x 0x%06lx 0x%016lx 0x%016lx 0x%06lx 0x%06lx %3x 0x%lx\n", h->p_type, h->p_offset, h->p_vaddr, h->p_paddr, h->p_filesz, h->p_memsz, h->p_flags, h->p_align);
  }
}

void elf_print_stab(elf_t *e) {
  struct Elf32_Shdr *h=&e->shdrs[e->hdr->e_shstrndx];
  printf("\nstring table:\n");
  for (int i=0; i<h->sh_size; i++) {
    char ch = e->stab[i];
    if (ch == '\0') ch = ' ';
    printf("%c", ch);
  }
  printf("\n");
}

void elf_print_header(elf_t *e) {
  elf_print_hdr(e);
  elf_print_shdrs(e);
  elf_print_phdrs(e);
  elf_print_stab(e);
}

void hex_dump(char *b, int len) {
  for (int i=0; i<len; i++) {
    printf("%02x ", (uint8_t) b[i]);
    if (i >= 200) {
      printf("...");
      break;
    }
  }
  printf("\n");
}

void elf_dump_section(elf_t *e, int si) {
  printf("%s\n", elf_str(e, e->shdrs[si].sh_name));
  char *s = elf_section(e, si);
  hex_dump(s, e->shdrs[si].sh_size);
  printf("\n");
}

void elf_dump_body(elf_t *e) {
  for (int si=0; si< e->hdr->e_shnum; si++) {
    elf_dump_section(e, si);
  }
}

bool elf_load(elf_t *e, const char *path)
{
    FILE *file = fopen(path, "rb");

    if (!file) return false;

    e->file = file;
    
    e->rawsize = fread(e->rawdata, 1, sizeof(e->rawdata), file);
    e->hdr = (struct Elf32_Hdr *)e->rawdata;

    if (!elf_valid(e)) return false;

    e->shdrs = (struct Elf32_Shdr*) &e->rawdata[e->hdr->e_shoff];
    e->phdrs = (struct Elf32_Phdr*) &e->rawdata[e->hdr->e_phoff];

    struct Elf32_Shdr *sh = &e->shdrs[e->hdr->e_shstrndx];
    e->stab = (char *) &e->rawdata[sh->sh_offset];

    elf_print_header(e);
    elf_dump_body(e);
    return true;
}
