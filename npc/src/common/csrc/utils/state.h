#ifndef NPC_STATE_H
#define NPC_STATE_H

#include <cstdint>

void npc_init_state();
void npc_start_timer();
uint64_t npc_get_uptime_us();
void npc_set_trap(int pc, int code);
bool npc_is_halted();
int npc_get_halt_pc();
int npc_get_halt_code();
int npc_report_exit_status();

#endif
