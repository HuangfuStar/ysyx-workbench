#include "trace/ftrace.h"

#ifdef CONFIG_FTRACE

#include <elf.h>
#include <cstdio>
#include <cstdlib>

namespace {

using Elf_Ehdr = Elf32_Ehdr;
using Elf_Shdr = Elf32_Shdr;
using Elf_Sym = Elf32_Sym;

struct FtraceFunc {
  uint32_t addr;
  uint32_t size;
  const char *name;
};

FtraceFunc *g_funcs = nullptr;
int g_nr_funcs = 0;
char *g_strtab = nullptr;
int g_call_depth = 0;

int sym_type(Elf_Sym *sym) {
  return ELF32_ST_TYPE(sym->st_info);
}

FtraceFunc *find_func(uint32_t addr) {
  for (int i = 0; i < g_nr_funcs; ++i) {
    uint32_t start = g_funcs[i].addr;
    uint32_t end = start + g_funcs[i].size;
    if (g_funcs[i].size == 0) {
      if (addr == start) {
        return &g_funcs[i];
      }
      continue;
    }
    if (addr >= start && addr < end) {
      return &g_funcs[i];
    }
  }
  return nullptr;
}

void print_indent(int depth) {
  for (int i = 0; i < depth; ++i) {
    std::printf("  ");
  }
}

int32_t sign_extend(uint32_t value, int bits) {
  const uint32_t shift = 32 - bits;
  return static_cast<int32_t>(value << shift) >> shift;
}

}  // namespace

void init_ftrace(const char *elf_file) {
  if (elf_file == nullptr) {
    std::fprintf(stderr, "ftrace requires an ELF file, please pass it with -e\n");
    std::exit(1);
  }

  FILE *fp = std::fopen(elf_file, "rb");
  if (fp == nullptr) {
    std::fprintf(stderr, "can not open ELF file '%s'\n", elf_file);
    std::exit(1);
  }

  Elf_Ehdr ehdr = {};
  size_t ret = std::fread(&ehdr, sizeof(ehdr), 1, fp);
  if (ret != 1 || ehdr.e_ident[EI_MAG0] != ELFMAG0 || ehdr.e_ident[EI_MAG1] != ELFMAG1 ||
      ehdr.e_ident[EI_MAG2] != ELFMAG2 || ehdr.e_ident[EI_MAG3] != ELFMAG3) {
    std::fprintf(stderr, "'%s' is not a valid ELF file\n", elf_file);
    std::exit(1);
  }

  Elf_Shdr *shdrs = static_cast<Elf_Shdr *>(std::malloc(ehdr.e_shentsize * ehdr.e_shnum));
  std::fseek(fp, ehdr.e_shoff, SEEK_SET);
  ret = std::fread(shdrs, ehdr.e_shentsize, ehdr.e_shnum, fp);
  if (ret != static_cast<size_t>(ehdr.e_shnum)) {
    std::fprintf(stderr, "can not read section headers from '%s'\n", elf_file);
    std::exit(1);
  }

  Elf_Shdr *symtab_hdr = nullptr;
  Elf_Shdr *strtab_hdr = nullptr;
  for (int i = 0; i < ehdr.e_shnum; ++i) {
    if (shdrs[i].sh_type == SHT_SYMTAB) {
      symtab_hdr = &shdrs[i];
      strtab_hdr = &shdrs[shdrs[i].sh_link];
      break;
    }
  }
  if (symtab_hdr == nullptr || strtab_hdr == nullptr) {
    std::fprintf(stderr, "can not find symbol table in '%s'\n", elf_file);
    std::exit(1);
  }

  g_strtab = static_cast<char *>(std::malloc(strtab_hdr->sh_size));
  std::fseek(fp, strtab_hdr->sh_offset, SEEK_SET);
  ret = std::fread(g_strtab, 1, strtab_hdr->sh_size, fp);
  if (ret != strtab_hdr->sh_size) {
    std::fprintf(stderr, "can not read string table from '%s'\n", elf_file);
    std::exit(1);
  }

  const int nr_syms = symtab_hdr->sh_size / sizeof(Elf_Sym);
  Elf_Sym *syms = static_cast<Elf_Sym *>(std::malloc(symtab_hdr->sh_size));
  std::fseek(fp, symtab_hdr->sh_offset, SEEK_SET);
  ret = std::fread(syms, sizeof(Elf_Sym), nr_syms, fp);
  if (ret != static_cast<size_t>(nr_syms)) {
    std::fprintf(stderr, "can not read symbol table from '%s'\n", elf_file);
    std::exit(1);
  }

  for (int i = 0; i < nr_syms; ++i) {
    if (sym_type(&syms[i]) == STT_FUNC && syms[i].st_name != 0 && syms[i].st_shndx != SHN_UNDEF) {
      ++g_nr_funcs;
    }
  }

  g_funcs = static_cast<FtraceFunc *>(std::malloc(sizeof(FtraceFunc) * g_nr_funcs));
  int idx = 0;
  for (int i = 0; i < nr_syms; ++i) {
    if (sym_type(&syms[i]) != STT_FUNC || syms[i].st_name == 0 || syms[i].st_shndx == SHN_UNDEF) {
      continue;
    }
    g_funcs[idx].addr = syms[i].st_value;
    g_funcs[idx].size = syms[i].st_size;
    g_funcs[idx].name = g_strtab + syms[i].st_name;
    ++idx;
  }

  std::free(syms);
  std::free(shdrs);
  std::fclose(fp);
}

void ftrace_on_inst(uint32_t pc, uint32_t inst, uint32_t next_pc) {
  const uint32_t opcode = inst & 0x7f;
  const uint32_t rd = (inst >> 7) & 0x1f;
  const uint32_t rs1 = (inst >> 15) & 0x1f;
  const int32_t imm_i = sign_extend(inst >> 20, 12);

  if (opcode == 0x6f) {
    if (rd == 1 || rd == 5) {
      FtraceFunc *func = find_func(next_pc);
      print_indent(g_call_depth);
      std::printf("[ftrace] call 0x%08x -> %s@0x%08x\n",
          pc, func ? func->name : "<unknown>", next_pc);
      ++g_call_depth;
    }
    return;
  }

  if (opcode == 0x67) {
    if (rd == 0 && imm_i == 0 && (rs1 == 1 || rs1 == 5)) {
      if (g_call_depth > 0) {
        --g_call_depth;
      }
      FtraceFunc *func = find_func(pc);
      print_indent(g_call_depth);
      std::printf("[ftrace] ret  %s@0x%08x -> 0x%08x\n",
          func ? func->name : "<unknown>", pc, next_pc);
    } else if (rd == 1 || rd == 5) {
      FtraceFunc *func = find_func(next_pc);
      print_indent(g_call_depth);
      std::printf("[ftrace] call 0x%08x -> %s@0x%08x\n",
          pc, func ? func->name : "<unknown>", next_pc);
      ++g_call_depth;
    }
  }
}

#else

void init_ftrace(const char *elf_file) {
  (void)elf_file;
}

void ftrace_on_inst(uint32_t pc, uint32_t inst, uint32_t next_pc) {
  (void)pc;
  (void)inst;
  (void)next_pc;
}

#endif
