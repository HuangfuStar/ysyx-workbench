#include <verilated.h>

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <vector>

#include "VTop.h"
#include "verilated_fst_c.h"

namespace {

constexpr uint32_t kPmemBase = 0x00000000u;
constexpr uint32_t kPmemSize = 1u << 20;
constexpr uint32_t kResetCycles = 5;
constexpr uint64_t kMaxCycles = 100000;
constexpr uint32_t kEbreakInst = 0x00100073u;

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

  dut.rst_in = 0;
  for (uint64_t cycle = 0; cycle < kMaxCycles && !g_halted; ++cycle) {
    tick(&dut, &trace, sim_time);
  }

  dut.final();
  trace.close();

  if (g_halted) {
    std::printf("ebreak at pc = 0x%08x, code = %d\n", static_cast<uint32_t>(g_halt_pc), g_halt_code);
    return g_halt_code;
  }

  std::fprintf(stderr, "simulation timeout after %llu cycles\n", static_cast<unsigned long long>(kMaxCycles));
  return 1;
}
