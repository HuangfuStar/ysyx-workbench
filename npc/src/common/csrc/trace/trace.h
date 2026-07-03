#ifndef NPC_TRACE_H
#define NPC_TRACE_H

#include <cstdint>

struct TraceEvent {
  uint32_t pc;
  uint32_t inst;
  uint32_t next_pc;
  bool mem_valid;
  bool mem_is_write;
  uint32_t mem_addr;
  uint32_t mem_wdata;
  uint32_t mem_rdata;
  uint8_t mem_len;
};

void trace_init(const char *elf_file);
void trace_on_step(const TraceEvent &event);
void trace_dump_iringbuf();

#endif
