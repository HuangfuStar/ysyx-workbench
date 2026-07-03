#include "utils/state.h"

#include <cstdint>
#include <cstdio>
#include <sys/time.h>

#include "device/device.h"
#include "trace/trace.h"

namespace {

uint64_t g_time_start_point = 0;
bool g_halted = false;
int g_halt_pc = 0;
int g_halt_code = 0;

uint64_t get_time_us() {
  struct timeval now = {};
  gettimeofday(&now, nullptr);
  return static_cast<uint64_t>(now.tv_sec) * 1000000 + now.tv_usec;
}

}  // namespace

void npc_init_state() {
  g_time_start_point = 0;
  g_halted = false;
  g_halt_pc = 0;
  g_halt_code = 0;
}

void npc_start_timer() {
  g_time_start_point = get_time_us();
}

uint64_t npc_get_uptime_us() {
  return get_time_us() - g_time_start_point;
}

void npc_set_trap(int pc, int code) {
  g_halted = true;
  g_halt_pc = pc;
  g_halt_code = code;
}

bool npc_is_halted() {
  return g_halted;
}

int npc_get_halt_pc() {
  return g_halt_pc;
}

int npc_get_halt_code() {
  return g_halt_code;
}

int npc_report_exit_status() {
  if (g_halted) {
#ifdef CONFIG_IRINGBUF
    trace_dump_iringbuf();
#endif
    device_trap_report(g_halt_pc, g_halt_code);
    return g_halt_code;
  }

  std::fprintf(stderr, "simulation stopped before trap\n");
  return 0;
}

extern "C" void npc_ebreak(int pc, int code) {
  npc_set_trap(pc, code);
}
