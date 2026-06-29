#include <verilated.h>

#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <sys/time.h>
#include <vector>

#include "VTop.h"
#include "verilated_fst_c.h"

namespace {

constexpr const char *kAnsiFgRed = "\33[1;31m";
constexpr const char *kAnsiFgGreen = "\33[1;32m";
constexpr const char *kAnsiNone = "\33[0m";

constexpr uint32_t kPmemBase = 0x80000000u;
constexpr uint32_t kPmemSize = 1u << 20;
constexpr uint32_t kUartAddr = 0x10000000u;
constexpr uint32_t kTimerAddr = 0x10000010u;
constexpr uint32_t kResetCycles = 5;
constexpr uint64_t kMaxCycles = 100000;
constexpr uint32_t kEbreakInst = 0x00100073u;

uint64_t time_start_point;

uint8_t g_pmem[kPmemSize] = {};
bool g_halted = false;
int g_halt_pc = 0;
int g_halt_code = 0;

bool in_pmem(uint32_t addr) {
  return addr >= kPmemBase && addr < (kPmemBase + kPmemSize);
}

uint32_t host_index(uint32_t addr) {
  return (addr & ~0x3u) - kPmemBase;
}

uint64_t get_time_us() {
  struct timeval now = {};
  gettimeofday(&now, nullptr);
  return static_cast<uint64_t>(now.tv_sec) * 1000000 + now.tv_usec;
}

uint64_t get_uptime_us() {
  return get_time_us() - time_start_point;
}

void init_default_image() {
  g_pmem[0] = static_cast<uint8_t>(kEbreakInst & 0xffu);
  g_pmem[1] = static_cast<uint8_t>((kEbreakInst >> 8) & 0xffu);
  g_pmem[2] = static_cast<uint8_t>((kEbreakInst >> 16) & 0xffu);
  g_pmem[3] = static_cast<uint8_t>((kEbreakInst >> 24) & 0xffu);
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

void tick(VTop *dut, VerilatedFstC *trace, vluint64_t &sim_time) {
  dut->clk_in = 0;
  dut->eval();
  trace->dump(sim_time);
  sim_time++;
  dut->clk_in = 1;
  dut->eval();
  trace->dump(sim_time);
  sim_time++;
}

}  // namespace

extern "C" int pmem_read(int raddr) {
  if (raddr == static_cast<int>(kTimerAddr)) {
    return static_cast<int>(get_uptime_us());
  }
  if (raddr == static_cast<int>(kTimerAddr + 4)) {
    return static_cast<int>(get_uptime_us() >> 32);
  }

  const uint32_t addr = static_cast<uint32_t>(raddr) & ~0x3u;
  if (!in_pmem(addr)) {
    std::fprintf(stderr, "pmem_read out of range: 0x%08x\n", addr);
    return 0;
  }

  const uint32_t index = host_index(addr);
  return static_cast<int>(g_pmem[index]) |
         (static_cast<int>(g_pmem[index + 1]) << 8) |
         (static_cast<int>(g_pmem[index + 2]) << 16) |
         (static_cast<int>(g_pmem[index + 3]) << 24);
}

extern "C" void pmem_write(int waddr, int wdata, char wmask) {
  if (waddr == static_cast<int>(kUartAddr)) {
    if (static_cast<uint8_t>(wmask) != 0x01) {
      std::fprintf(stderr, "uart only supports 8-bit writes: addr=0x%08x mask=0x%02x\n",
          static_cast<uint32_t>(waddr), static_cast<uint8_t>(wmask));
      assert(0);
    }
    std::putchar(wdata & 0xff);
    std::fflush(stdout);
    return;
  }

  if (waddr == static_cast<int>(kTimerAddr) || waddr == static_cast<int>(kTimerAddr + 4)) {
    if (static_cast<uint8_t>(wmask) != 0x0f) {
      std::fprintf(stderr, "timer only supports 32-bit accesses: addr=0x%08x mask=0x%02x\n",
          static_cast<uint32_t>(waddr), static_cast<uint8_t>(wmask));
      std::exit(1);
    }
    std::fprintf(stderr, "timer is read-only: 0x%08x\n", static_cast<uint32_t>(waddr));
    std::exit(1);
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

extern "C" void npc_ebreak(int pc, int code) {
  g_halted = true;
  g_halt_pc = pc;
  g_halt_code = code;
}

int main(int argc, char **argv) {
  Verilated::commandArgs(argc, argv);
  Verilated::traceEverOn(true);
  load_image(argc > 1 ? argv[1] : nullptr);

  VTop dut;
  VerilatedFstC trace;
  vluint64_t sim_time = 0;

  dut.trace(&trace, 99);
  trace.open("build/wave.fst");

  dut.rst_in = 1;
  for (uint32_t i = 0; i < kResetCycles; ++i) {
    tick(&dut, &trace, sim_time);
  }

  time_start_point = get_time_us();
  dut.rst_in = 0;
  for (uint64_t cycle = 0; !g_halted; ++cycle) {
    tick(&dut, &trace, sim_time);
  }

  dut.final();
  trace.close();

  if (g_halted) {
    const char *trap_name = (g_halt_code == 0) ? "HIT GOOD TRAP" : "HIT BAD TRAP";
    const char *trap_color = (g_halt_code == 0) ? kAnsiFgGreen : kAnsiFgRed;
    std::printf("npc: %s%s%s at pc = 0x%08x, code = %d\n",
        trap_color, trap_name, kAnsiNone, static_cast<uint32_t>(g_halt_pc), g_halt_code);
    return g_halt_code;
  }

  std::fprintf(stderr, "simulation timeout after %llu cycles\n", static_cast<unsigned long long>(kMaxCycles));
  return 1;
}
