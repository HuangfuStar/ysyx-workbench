#ifndef NPC_TIMER_H
#define NPC_TIMER_H

#include <cstdint>

bool timer_read(uint32_t addr, uint32_t *data);
bool timer_write(uint32_t addr, uint32_t data, uint8_t mask);

#endif
