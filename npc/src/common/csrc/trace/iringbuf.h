#ifndef NPC_IRINGBUF_H
#define NPC_IRINGBUF_H

#include <cstdint>

void iringbuf_add(uint32_t pc, uint32_t inst);
void iringbuf_dump();

#endif
