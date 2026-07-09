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

#include <isa.h>
/** 
 * @brief  simulate the trap(interrupt or excetption)
 * 
 * @param  NO   Exception code
 * @param  epc
 *
 * @return
 */
word_t isa_raise_intr(word_t NO, vaddr_t epc) {
  cpu.mepc = epc;
  cpu.mcause = NO;
  IFDEF(CONFIG_ETRACE,
    if (NO == 11) {
      log_write("[etrace] intr NO=%lu mepc=" FMT_WORD " mtvec=" FMT_WORD
          " " MUXDEF(CONFIG_RVE, "a5", "a7") "=" FMT_WORD "\n",
          (unsigned long)NO, cpu.mepc, cpu.mtvec,
          cpu.gpr[MUXDEF(CONFIG_RVE, 15, 17)]);
    }
    else {
      log_write("[etrace] intr NO=%lu mepc=" FMT_WORD " mtvec=" FMT_WORD "\n",
          (unsigned long)NO, cpu.mepc, cpu.mtvec);
    }
  );
  return cpu.mtvec;
}

word_t isa_query_intr() {
  return INTR_EMPTY;
}
