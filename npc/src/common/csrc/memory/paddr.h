#ifndef NPC_PADDR_H
#define NPC_PADDR_H

#include <cstdint>

void init_default_image();
void load_image(const char *img_file);
uint32_t pmem_read_word(uint32_t addr);

#endif
