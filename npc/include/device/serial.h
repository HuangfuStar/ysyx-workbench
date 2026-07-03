#ifndef NPC_SERIAL_H
#define NPC_SERIAL_H

#include <cstdint>

bool serial_write(uint32_t addr, uint32_t data, uint8_t mask);

#endif
