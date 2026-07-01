#include <cpu/ftrace.h>

#include <elf.h>
#include <stdio.h>
#include <stdlib.h>

typedef MUXDEF(CONFIG_ISA64, Elf64_Ehdr, Elf32_Ehdr) Elf_Ehdr;
typedef MUXDEF(CONFIG_ISA64, Elf64_Shdr, Elf32_Shdr) Elf_Shdr;
typedef MUXDEF(CONFIG_ISA64, Elf64_Sym, Elf32_Sym) Elf_Sym;

typedef struct {
  vaddr_t addr;
  word_t size;
  const char *name;
} ftrace_func_t;

static ftrace_func_t *funcs = NULL;
static int nr_funcs = 0;
static char *strtab = NULL;
static int call_depth = 0;

static int sym_type(Elf_Sym *sym) {
  return MUXDEF(CONFIG_ISA64, ELF64_ST_TYPE(sym->st_info), ELF32_ST_TYPE(sym->st_info));
}

static ftrace_func_t *find_func(vaddr_t addr) {
  for (int i = 0; i < nr_funcs; i++) {
    vaddr_t start = funcs[i].addr;
    vaddr_t end = start + funcs[i].size;
    if (funcs[i].size == 0) {
      if (addr == start) {
        return &funcs[i];
      }
      continue;
    }
    if (addr >= start && addr < end) {
      return &funcs[i];
    }
  }
  return NULL;
}

static void print_indent(int depth) {
  for (int i = 0; i < depth; i++) {
    log_write("  ");
  }
}

void init_ftrace(const char *elf_file) {
  Assert(elf_file != NULL, "ftrace requires an ELF file, please pass it with -e/--elf");

  FILE *fp = fopen(elf_file, "rb");
  Assert(fp != NULL, "Can not open ELF file '%s'", elf_file);

  Elf_Ehdr ehdr = {};
  size_t ret = fread(&ehdr, sizeof(ehdr), 1, fp);
  Assert(ret == 1, "Can not read ELF header from '%s'", elf_file);
  Assert(ehdr.e_ident[EI_MAG0] == ELFMAG0 &&
         ehdr.e_ident[EI_MAG1] == ELFMAG1 &&
         ehdr.e_ident[EI_MAG2] == ELFMAG2 &&
         ehdr.e_ident[EI_MAG3] == ELFMAG3,
      "'%s' is not a valid ELF file", elf_file);

  Elf_Shdr *shdrs = malloc(ehdr.e_shentsize * ehdr.e_shnum);
  Assert(shdrs != NULL, "Can not allocate section headers for '%s'", elf_file);
  ret = fseek(fp, ehdr.e_shoff, SEEK_SET);
  Assert(ret == 0, "Can not seek section headers in '%s'", elf_file);
  ret = fread(shdrs, ehdr.e_shentsize, ehdr.e_shnum, fp);
  Assert(ret == ehdr.e_shnum, "Can not read section headers from '%s'", elf_file);

  Elf_Shdr *symtab_hdr = NULL;
  Elf_Shdr *strtab_hdr = NULL;
  for (int i = 0; i < ehdr.e_shnum; i++) {
    if (shdrs[i].sh_type == SHT_SYMTAB) {
      symtab_hdr = &shdrs[i];
      Assert(shdrs[i].sh_link < ehdr.e_shnum, "Invalid string table index in '%s'", elf_file);
      strtab_hdr = &shdrs[shdrs[i].sh_link];
      break;
    }
  }
  Assert(symtab_hdr != NULL && strtab_hdr != NULL, "Can not find symbol table in '%s'", elf_file);

  strtab = malloc(strtab_hdr->sh_size);
  Assert(strtab != NULL, "Can not allocate string table for '%s'", elf_file);
  ret = fseek(fp, strtab_hdr->sh_offset, SEEK_SET);
  Assert(ret == 0, "Can not seek string table in '%s'", elf_file);
  ret = fread(strtab, 1, strtab_hdr->sh_size, fp);
  Assert(ret == strtab_hdr->sh_size, "Can not read string table from '%s'", elf_file);

  int nr_syms = symtab_hdr->sh_size / sizeof(Elf_Sym);
  Elf_Sym *syms = malloc(symtab_hdr->sh_size);
  Assert(syms != NULL, "Can not allocate symbol table for '%s'", elf_file);
  ret = fseek(fp, symtab_hdr->sh_offset, SEEK_SET);
  Assert(ret == 0, "Can not seek symbol table in '%s'", elf_file);
  ret = fread(syms, sizeof(Elf_Sym), nr_syms, fp);
  Assert(ret == nr_syms, "Can not read symbol table from '%s'", elf_file);

  for (int i = 0; i < nr_syms; i++) {
    if (sym_type(&syms[i]) == STT_FUNC && syms[i].st_name != 0 && syms[i].st_shndx != SHN_UNDEF) {
      nr_funcs++;
    }
  }

  funcs = malloc(sizeof(ftrace_func_t) * nr_funcs);
  Assert(funcs != NULL || nr_funcs == 0, "Can not allocate ftrace symbols for '%s'", elf_file);

  int idx = 0;
  for (int i = 0; i < nr_syms; i++) {
    if (sym_type(&syms[i]) != STT_FUNC || syms[i].st_name == 0 || syms[i].st_shndx == SHN_UNDEF) {
      continue;
    }
    funcs[idx].addr = syms[i].st_value;
    funcs[idx].size = syms[i].st_size;
    funcs[idx].name = strtab + syms[i].st_name;
    idx++;
  }

  free(syms);
  free(shdrs);
  fclose(fp);

  Log("ftrace loaded %d function symbols from %s", nr_funcs, elf_file);
}

void ftrace_call(vaddr_t pc, vaddr_t target) {
  ftrace_func_t *func = find_func(target);
  print_indent(call_depth);
  log_write("[ftrace] call " FMT_WORD " -> %s@" FMT_WORD "\n",
      pc, func ? func->name : "<unknown>", target);
  call_depth++;
}

void ftrace_ret(vaddr_t pc, vaddr_t target) {
  if (call_depth > 0) {
    call_depth--;
  }
  ftrace_func_t *func = find_func(pc);
  print_indent(call_depth);
  log_write("[ftrace] ret  %s@" FMT_WORD " -> " FMT_WORD "\n",
      func ? func->name : "<unknown>", pc, target);
}
