#include "trace/trace.h"

#include <cstdio>

#include "trace/disasm.h"
#include "trace/ftrace.h"
#include "trace/iringbuf.h"

void trace_init(const char *elf_file) {
#if defined(CONFIG_ITRACE) || defined(CONFIG_IRINGBUF)
  init_disasm();
#endif
#ifdef CONFIG_FTRACE
  init_ftrace(elf_file);
#else
  (void)elf_file;
#endif
}

void trace_on_step(const TraceEvent &event) {
#ifdef CONFIG_IRINGBUF
  iringbuf_add(event.pc, event.inst);
#endif

#ifdef CONFIG_ITRACE
  char asm_buf[128] = {};
  disassemble_inst(asm_buf, sizeof(asm_buf), event.pc, event.inst);
  std::printf("[itrace] 0x%08x: %08x  %s\n", event.pc, event.inst, asm_buf);
#endif

#ifdef CONFIG_MTRACE
  if (event.mem_valid) {
    if (event.mem_is_write) {
      std::printf("[mtrace] W pc=0x%08x addr=0x%08x len=%u data=0x%08x\n",
          event.pc, event.mem_addr, event.mem_len, event.mem_wdata);
    } else {
      std::printf("[mtrace] R pc=0x%08x addr=0x%08x len=%u data=0x%08x\n",
          event.pc, event.mem_addr, event.mem_len, event.mem_rdata);
    }
  }
#endif

#ifdef CONFIG_FTRACE
  ftrace_on_inst(event.pc, event.inst, event.next_pc);
#endif
}

void trace_dump_iringbuf() {
  iringbuf_dump();
}
