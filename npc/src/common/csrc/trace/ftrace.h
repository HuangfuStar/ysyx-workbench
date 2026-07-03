#ifndef NPC_FTRACE_H
#define NPC_FTRACE_H

#include <cstdint>

void init_ftrace(const char *elf_file);
void ftrace_on_inst(uint32_t pc, uint32_t inst, uint32_t next_pc);

#endif
