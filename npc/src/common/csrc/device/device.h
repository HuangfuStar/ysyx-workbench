#ifndef NPC_DEVICE_H
#define NPC_DEVICE_H

#include <cstdint>

void init_device();
bool device_read(uint32_t addr, uint32_t *data);
bool device_write(uint32_t addr, uint32_t data, uint8_t mask);
void device_trap_report(int pc, int code);

#endif
