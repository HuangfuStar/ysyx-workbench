#include "device/serial.h"

#include <cassert>
#include <cstdint>
#include <cstdio>

namespace {

constexpr uint32_t kUartAddr = 0x10000000u;

}  // namespace

bool serial_write(uint32_t addr, uint32_t data, uint8_t mask) {
  if (addr != kUartAddr) {
    return false;
  }
  if (mask != 0x01) {
    std::fprintf(stderr, "uart only supports 8-bit writes: addr=0x%08x mask=0x%02x\n",
        addr, mask);
    assert(0);
  }
  std::putchar(data & 0xffu);
  std::fflush(stdout);
  return true;
}
