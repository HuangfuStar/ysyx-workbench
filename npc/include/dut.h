#ifndef NPC_DUT_H
#define NPC_DUT_H

#include <cstdint>

struct TraceEvent;

void sim_init_dut(const char *wave_file);
void sim_finish_dut();
void sim_exec(uint64_t n);
bool sim_is_halted();
int sim_get_halt_pc();
int sim_get_halt_code();
uint32_t sim_get_pc();
uint32_t sim_get_gpr(int idx);
int sim_get_gpr_num();
const char *sim_get_gpr_name(int idx);
bool sim_get_reg_value(const char *name, uint32_t *value);
uint32_t sim_pmem_read_word(uint32_t addr);
bool sim_get_trace_event(TraceEvent *event);

#endif
