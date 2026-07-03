#include "device/timer.h"

#include <cstdint>
#include <cstdio>
#include <cstdlib>

#include "utils/state.h"

namespace {

constexpr uint32_t kTimerAddr = 0x10000010u;

}  // namespace

bool timer_read(uint32_t addr, uint32_t *data) {
  if (data == nullptr) {
    return false;
  }
  if (addr == kTimerAddr) {
    *data = static_cast<uint32_t>(npc_get_uptime_us());
    return true;
  }
  if (addr == kTimerAddr + 4) {
    *data = static_cast<uint32_t>(npc_get_uptime_us() >> 32);
    return true;
  }
  return false;
}

bool timer_write(uint32_t addr, uint32_t data, uint8_t mask) {
  (void)data;
  if (addr != kTimerAddr && addr != kTimerAddr + 4) {
    return false;
  }
  if (mask != 0x0f) {
    std::fprintf(stderr, "timer only supports 32-bit accesses: addr=0x%08x mask=0x%02x\n",
        addr, mask);
    std::exit(1);
  }
  std::fprintf(stderr, "timer is read-only: 0x%08x\n", addr);
  std::exit(1);
}
