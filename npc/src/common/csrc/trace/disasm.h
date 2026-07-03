#ifndef NPC_DISASM_H
#define NPC_DISASM_H

#include <cstdint>

void init_disasm();
void disassemble_inst(char *str, int size, uint64_t pc, uint32_t inst);

#endif
