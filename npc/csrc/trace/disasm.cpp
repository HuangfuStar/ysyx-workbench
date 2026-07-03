#include "trace/disasm.h"

#include <cstdio>

#if defined(CONFIG_ITRACE) || defined(CONFIG_IRINGBUF)

#include <cassert>
#include <dlfcn.h>

#include <capstone/capstone.h>

#if defined(__APPLE__)
#define CS_LIB_SUFFIX "5.dylib"
#elif defined(__linux__)
#define CS_LIB_SUFFIX "so.5"
#else
#error "Unsupported platform"
#endif

namespace {

size_t (*g_cs_disasm)(csh handle, const uint8_t *code,
    size_t code_size, uint64_t address, size_t count, cs_insn **insn) = nullptr;
void (*g_cs_free)(cs_insn *insn, size_t count) = nullptr;
csh g_handle = 0;
bool g_inited = false;

}  // namespace

void init_disasm() {
  if (g_inited) {
    return;
  }

  void *dl_handle = dlopen("../nemu/tools/capstone/repo/libcapstone." CS_LIB_SUFFIX, RTLD_LAZY);
  assert(dl_handle != nullptr);

  auto cs_open_dl = reinterpret_cast<cs_err (*)(cs_arch, cs_mode, csh *)>(dlsym(dl_handle, "cs_open"));
  assert(cs_open_dl != nullptr);

  g_cs_disasm = reinterpret_cast<size_t (*)(csh, const uint8_t *, size_t, uint64_t, size_t, cs_insn **)>(
      dlsym(dl_handle, "cs_disasm"));
  assert(g_cs_disasm != nullptr);

  g_cs_free = reinterpret_cast<void (*)(cs_insn *, size_t)>(dlsym(dl_handle, "cs_free"));
  assert(g_cs_free != nullptr);

  cs_err ret = cs_open_dl(CS_ARCH_RISCV, CS_MODE_RISCV32, &g_handle);
  assert(ret == CS_ERR_OK);
  g_inited = true;
}

void disassemble_inst(char *str, int size, uint64_t pc, uint32_t inst) {
  if (!g_inited) {
    std::snprintf(str, size, "<disasm disabled>");
    return;
  }

  uint8_t code[4] = {};
  for (int i = 0; i < 4; ++i) {
    code[i] = static_cast<uint8_t>((inst >> (i * 8)) & 0xffu);
  }

  cs_insn *insn = nullptr;
  size_t count = g_cs_disasm(g_handle, code, sizeof(code), pc, 0, &insn);
  assert(count == 1);
  int ret = std::snprintf(str, size, "%s", insn->mnemonic);
  if (insn->op_str[0] != '\0') {
    std::snprintf(str + ret, size - ret, "\t%s", insn->op_str);
  }
  g_cs_free(insn, count);
}

#else

void init_disasm() {
}

void disassemble_inst(char *str, int size, uint64_t pc, uint32_t inst) {
  (void)pc;
  (void)inst;
  std::snprintf(str, size, "<disasm disabled>");
}

#endif
