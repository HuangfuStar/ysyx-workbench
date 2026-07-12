#include "trace/trace.h"

#include <cstdio>

#include "common.h"
#include "trace/disasm.h"
#include "trace/ftrace.h"
#include "trace/iringbuf.h"
#include "utils/log.h"

void trace_init(const char *elf_file) {
  IFDEF(CONFIG_ITRACE, init_disasm();)
  IFNDEF(CONFIG_ITRACE, IFDEF(CONFIG_IRINGBUF, init_disasm();))
  IFDEF(CONFIG_FTRACE, init_ftrace(elf_file);)
  IFNDEF(CONFIG_FTRACE, (void)elf_file;)
}

void trace_on_step(const TraceEvent &event) {
  IFDEF(CONFIG_IRINGBUF, iringbuf_add(event.pc, event.inst);)

  IFDEF(CONFIG_ITRACE,
  char asm_buf[128] = {};
  disassemble_inst(asm_buf, sizeof(asm_buf), event.pc, event.inst);
  log_write("[itrace] 0x%08x: %08x  %s\n", event.pc, event.inst, asm_buf);
  )

  IFDEF(CONFIG_MTRACE,
  if (event.mem_valid) {
    if (event.mem_is_write) {
      log_write("[mtrace] W pc=0x%08x addr=0x%08x len=%u data=0x%08x\n",
          event.pc, event.mem_addr, event.mem_len, event.mem_wdata);
    } else {
      log_write("[mtrace] R pc=0x%08x addr=0x%08x len=%u data=0x%08x\n",
          event.pc, event.mem_addr, event.mem_len, event.mem_rdata);
    }
  }
  )

  IFDEF(CONFIG_FTRACE, ftrace_on_inst(event.pc, event.inst, event.next_pc);)
}

void trace_dump_iringbuf() {
  iringbuf_dump();
}
