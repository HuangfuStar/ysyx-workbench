/***************************************************************************************
* Copyright (c) 2014-2024 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#ifndef __RISCV_REG_H__
#define __RISCV_REG_H__

#include <isa.h>
#include <debug.h>

static inline int check_reg_idx(int idx) {
  IFDEF(CONFIG_RT_CHECK, Assert(idx >= 0 && idx < MUXDEF(CONFIG_RVE, 16, 32), "idx = %d", idx));
  return idx;
}

#define gpr(idx) (cpu.gpr[check_reg_idx(idx)])

enum {
  CSR_MSTATUS = 0x300,
  CSR_MTVEC   = 0x305,
  CSR_MEPC    = 0x341,
  CSR_MCAUSE  = 0x342,
};

static inline word_t *csr_ref(int csr) {
  switch (csr) {
    case CSR_MSTATUS: return &cpu.mstatus;
    case CSR_MTVEC:   return &cpu.mtvec;
    case CSR_MEPC:    return &cpu.mepc;
    case CSR_MCAUSE:  return &cpu.mcause;
    default: panic("unsupported CSR = 0x%x", csr);
  }
}

static inline word_t csr_read(int csr) {
  return *csr_ref(csr);
}

static inline void csr_write(int csr, word_t val) {
  *csr_ref(csr) = val;
}

static inline const char* reg_name(int idx) {
  extern const char* regs[];
  return regs[check_reg_idx(idx)];
}

#endif
