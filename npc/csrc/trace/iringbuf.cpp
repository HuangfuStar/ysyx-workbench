#include "trace/iringbuf.h"

#include <cstdio>

#include "trace/disasm.h"

#ifdef CONFIG_IRINGBUF

namespace {

struct IRingBufEntry {
  uint32_t pc;
  uint32_t inst;
  bool valid;
};

IRingBufEntry g_buf[CONFIG_IRINGBUF_NR] = {};
int g_head = 0;
int g_count = 0;

}  // namespace

void iringbuf_add(uint32_t pc, uint32_t inst) {
  g_buf[g_head] = {pc, inst, true};
  g_head = (g_head + 1) % CONFIG_IRINGBUF_NR;
  if (g_count < CONFIG_IRINGBUF_NR) {
    ++g_count;
  }
}

void iringbuf_dump() {
  if (g_count == 0) {
    std::puts("Instruction ring buffer is empty.");
    return;
  }

  std::puts("Instruction ring buffer:");
  const int start = (g_head - g_count + CONFIG_IRINGBUF_NR) % CONFIG_IRINGBUF_NR;
  for (int i = 0; i < g_count; ++i) {
    const int idx = (start + i) % CONFIG_IRINGBUF_NR;
    if (!g_buf[idx].valid) {
      continue;
    }
    char asm_buf[128] = {};
    disassemble_inst(asm_buf, sizeof(asm_buf), g_buf[idx].pc, g_buf[idx].inst);
    std::printf("%c 0x%08x: %s\n", (i == g_count - 1) ? '>' : ' ', g_buf[idx].pc, asm_buf);
  }
}

#else

void iringbuf_add(uint32_t pc, uint32_t inst) {
  (void)pc;
  (void)inst;
}

void iringbuf_dump() {
  std::puts("Instruction ring buffer is disabled.");
}

#endif
