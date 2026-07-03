#include <dut.h>

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <limits>

#include <verilated.h>

#include "VTop.h"
#include "VTop__Dpi.h"
#include "svdpi.h"
#include "verilated_fst_c.h"
#include "memory.h"
#include "trace/trace.h"
#include "utils/state.h"

namespace {

constexpr uint32_t kResetCycles = 5;

class DUT {
 public:
  explicit DUT(const char *wave_file) : sim_time_(0) {
    dut_.trace(&trace_, 99);
    trace_.open(wave_file);
    dut_.rst_in = 1;
    for (uint32_t i = 0; i < kResetCycles; ++i) {
      Tick();
    }
    dut_.rst_in = 0;
  }

  ~DUT() {
    dut_.final();
    trace_.close();
  }

  void Execute(uint64_t n) {
    while (n-- != 0 && !npc_is_halted()) {
      Tick();
    }
  }

 private:
  VTop dut_;
  VerilatedFstC trace_;
  vluint64_t sim_time_;

  void Tick() {
    dut_.clk_in = 0;
    dut_.eval();
#ifdef CONFIG_DEBUG
    TraceEvent event = {};
    if (!dut_.rst_in && sim_get_trace_event(&event)) {
      trace_on_step(event);
    }
#endif
    trace_.dump(sim_time_++);
    dut_.clk_in = 1;
    dut_.eval();
    trace_.dump(sim_time_++);
  }
};

DUT *g_dut = nullptr;
svScope g_debug_scope = nullptr;

const char *kGprNames32[] = {
    "zero", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
    "s0",   "s1", "a0", "a1", "a2", "a3", "a4", "a5",
    "a6",   "a7", "s2", "s3", "s4", "s5", "s6", "s7",
    "s8",   "s9", "s10", "s11", "t3", "t4", "t5", "t6",
};

const char *kGprNames16[] = {
    "zero", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
    "s0",   "s1", "a0", "a1", "a2", "a3", "a4", "a5",
};

svScope find_debug_scope() {
  const char *kScopeCandidates[] = {
      "TOP.u_core",
      "TOP.Top.u_core",
      "TOP.rootp.Top.u_core",
  };

  for (const char *name : kScopeCandidates) {
    svScope scope = svGetScopeFromName(name);
    if (scope != nullptr) {
      return scope;
    }
  }
  return nullptr;
}

}  // namespace

void sim_init_dut(const char *wave_file) {
  delete g_dut;
  g_dut = new DUT(wave_file);
#ifdef CONFIG_DEBUG
  g_debug_scope = find_debug_scope();
  if (g_debug_scope == nullptr) {
    std::fprintf(stderr, "failed to locate debug scope for exported DPI functions\n");
    std::exit(1);
  }
#endif
}

void sim_finish_dut() {
  delete g_dut;
  g_dut = nullptr;
}

void sim_exec(uint64_t n) {
  if (g_dut == nullptr) {
    return;
  }
  if (n == std::numeric_limits<uint64_t>::max()) {
    while (!npc_is_halted()) {
      g_dut->Execute(std::numeric_limits<uint64_t>::max());
    }
  } else {
    g_dut->Execute(n);
  }
}

bool sim_is_halted() {
  return npc_is_halted();
}

int sim_get_halt_pc() {
  return npc_get_halt_pc();
}

int sim_get_halt_code() {
  return npc_get_halt_code();
}

uint32_t sim_get_pc() {
  if (g_dut == nullptr) {
    return 0;
  }
#ifdef CONFIG_DEBUG
  svSetScope(g_debug_scope);
  return static_cast<uint32_t>(npc_get_pc());
#else
  return 0;
#endif
}

uint32_t sim_get_gpr(int idx) {
  if (g_dut == nullptr) {
    return 0;
  }
#ifdef CONFIG_DEBUG
  svSetScope(g_debug_scope);
  return static_cast<uint32_t>(npc_get_gpr(idx));
#else
  return 0;
#endif
}

int sim_get_gpr_num() {
#ifdef CONFIG_DEBUG
  if (g_dut == nullptr) {
    return 0;
  }
  svSetScope(g_debug_scope);
  return npc_get_gpr_num();
#else
  return 0;
#endif
}

const char *sim_get_gpr_name(int idx) {
  const int gpr_num = sim_get_gpr_num();
  const char *const *names = (gpr_num == 16) ? kGprNames16 : kGprNames32;
  if (idx < 0 || idx >= gpr_num) {
    return "inv";
  }
  return names[idx];
}

bool sim_get_reg_value(const char *name, uint32_t *value) {
  if (name == nullptr || value == nullptr) {
    return false;
  }
  if (std::strcmp(name, "pc") == 0) {
    *value = sim_get_pc();
    return true;
  }

  if (name[0] == 'x') {
    char *end = nullptr;
    const long idx = std::strtol(name + 1, &end, 10);
    if (end != nullptr && *end == '\0' && idx >= 0 && idx < sim_get_gpr_num()) {
      *value = sim_get_gpr(static_cast<int>(idx));
      return true;
    }
  }

  const int gpr_num = sim_get_gpr_num();
  const char *const *names = (gpr_num == 16) ? kGprNames16 : kGprNames32;
  for (int i = 0; i < gpr_num; ++i) {
    if (std::strcmp(name, names[i]) == 0) {
      *value = sim_get_gpr(i);
      return true;
    }
  }
  if (std::strcmp(name, "fp") == 0 && gpr_num > 8) {
    *value = sim_get_gpr(8);
    return true;
  }
  return false;
}

uint32_t sim_pmem_read_word(uint32_t addr) {
  return pmem_read_word(addr);
}

bool sim_get_trace_event(TraceEvent *event) {
#ifdef CONFIG_DEBUG
  if (g_dut == nullptr || event == nullptr) {
    return false;
  }
  svSetScope(g_debug_scope);
  event->pc = static_cast<uint32_t>(npc_get_pc());
  event->inst = static_cast<uint32_t>(npc_get_inst());
  event->next_pc = static_cast<uint32_t>(npc_get_next_pc());
  event->mem_valid = npc_get_mem_valid() != 0;
  event->mem_is_write = npc_get_mem_is_write() != 0;
  event->mem_addr = static_cast<uint32_t>(npc_get_mem_addr());
  event->mem_wdata = static_cast<uint32_t>(npc_get_mem_wdata());
  event->mem_rdata = static_cast<uint32_t>(npc_get_mem_rdata());
  event->mem_len = static_cast<uint8_t>(npc_get_mem_len());
  return true;
#else
  (void)event;
  return false;
#endif
}
