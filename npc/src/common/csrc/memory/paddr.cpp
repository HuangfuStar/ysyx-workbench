#include "memory/paddr.h"

#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <fstream>

#include "device/device.h"

namespace {

constexpr uint32_t kPmemBase = 0x80000000u;
constexpr uint32_t kPmemSize = 1u << 20;
constexpr uint32_t kDefaultImg[] = {
  0x00000297u,
  0x00028823u,
  0x0102c503u,
  0x00100073u,
  0xdeadbeefu,
};

uint8_t g_pmem[kPmemSize] = {};

bool in_pmem(uint32_t addr) {
  return addr >= kPmemBase && addr < (kPmemBase + kPmemSize);
}

uint32_t host_index(uint32_t addr) {
  return (addr & ~0x3u) - kPmemBase;
}

}  // namespace

void init_default_image() {
  for (size_t i = 0; i < sizeof(kDefaultImg) / sizeof(kDefaultImg[0]); ++i) {
    const uint32_t word = kDefaultImg[i];
    const uint32_t base = static_cast<uint32_t>(i) * 4;
    g_pmem[base + 0] = static_cast<uint8_t>(word & 0xffu);
    g_pmem[base + 1] = static_cast<uint8_t>((word >> 8) & 0xffu);
    g_pmem[base + 2] = static_cast<uint8_t>((word >> 16) & 0xffu);
    g_pmem[base + 3] = static_cast<uint8_t>((word >> 24) & 0xffu);
  }
}

void load_image(const char *img_file) {
  if (img_file == nullptr) {
    init_default_image();
    return;
  }

  std::ifstream input(img_file, std::ios::binary);
  if (!input) {
    std::fprintf(stderr, "failed to open image: %s\n", img_file);
    std::exit(1);
  }

  input.read(reinterpret_cast<char *>(g_pmem), kPmemSize);
  const std::streamsize image_size = input.gcount();
  if (image_size <= 0) {
    std::fprintf(stderr, "empty image: %s\n", img_file);
    std::exit(1);
  }
}

uint32_t pmem_read_word(uint32_t addr) {
  const uint32_t index = host_index(addr);
  return static_cast<uint32_t>(g_pmem[index]) |
         (static_cast<uint32_t>(g_pmem[index + 1]) << 8) |
         (static_cast<uint32_t>(g_pmem[index + 2]) << 16) |
         (static_cast<uint32_t>(g_pmem[index + 3]) << 24);
}

extern "C" int pmem_read(int raddr) {
  uint32_t device_data = 0;
  if (device_read(static_cast<uint32_t>(raddr), &device_data)) {
    return static_cast<int>(device_data);
  }

  const uint32_t addr = static_cast<uint32_t>(raddr) & ~0x3u;
  if (!in_pmem(addr)) {
    std::fprintf(stderr, "pmem_read out of range: 0x%08x\n", addr);
    return 0;
  }

  return static_cast<int>(pmem_read_word(addr));
}

extern "C" void pmem_write(int waddr, int wdata, char wmask) {
  if (device_write(static_cast<uint32_t>(waddr), static_cast<uint32_t>(wdata),
      static_cast<uint8_t>(wmask))) {
    return;
  }

  const uint32_t addr = static_cast<uint32_t>(waddr) & ~0x3u;
  if (!in_pmem(addr)) {
    std::fprintf(stderr, "pmem_write out of range: 0x%08x\n", addr);
    std::exit(1);
  }

  const uint32_t index = host_index(addr);
  const uint32_t data = static_cast<uint32_t>(wdata);
  const uint8_t mask = static_cast<uint8_t>(wmask);

  for (int i = 0; i < 4; ++i) {
    if ((mask >> i) & 0x1u) {
      g_pmem[index + i] = static_cast<uint8_t>((data >> (i * 8)) & 0xffu);
    }
  }
}
