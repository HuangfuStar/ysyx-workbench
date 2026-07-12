#include "device/device.h"

#include <cstdint>
#include <cstdio>

#include "device/serial.h"
#include "device/timer.h"
#include "utils/log.h"

namespace {

constexpr const char *kAnsiFgRed = "\33[1;31m";
constexpr const char *kAnsiFgGreen = "\33[1;32m";
constexpr const char *kAnsiNone = "\33[0m";

}  // namespace

void init_device() {
}

bool device_read(uint32_t addr, uint32_t *data) {
  return timer_read(addr, data);
}

bool device_write(uint32_t addr, uint32_t data, uint8_t mask) {
  if (serial_write(addr, data, mask)) {
    return true;
  }
  if (timer_write(addr, data, mask)) {
    return true;
  }
  return false;
}

void device_trap_report(int pc, int code) {
  const char *trap_name = (code == 0) ? "HIT GOOD TRAP" : "HIT BAD TRAP";
  const char *trap_color = (code == 0) ? kAnsiFgGreen : kAnsiFgRed;
  _Log("npc: %s%s%s at pc = 0x%08x, code = %d\n",
      trap_color, trap_name, kAnsiNone, static_cast<uint32_t>(pc), code);
}
