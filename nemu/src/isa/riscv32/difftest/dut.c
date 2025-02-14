/***************************************************************************************
 * Copyright (c) 2014-2022 Zihao Yu, Nanjing University
 *
 * NEMU is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan
 *PSL v2. You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY
 *KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
 *NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 *
 * See the Mulan PSL v2 for more details.
 ***************************************************************************************/

#include "../local-include/reg.h"
#include <cpu/difftest.h>
#include <isa.h>

#define CHECKDIFF_CSRS(p) if (ref_r->csrs.p != cpu.csrs.p) { \
  printf("CRS: difftest fail at " #p ", expect %#x got %#x\n", ref_r->csrs.p, cpu.csrs.p); \
  return false; \
}
#define CHECKDIFF_FMT(p, fmt, ...) if (ref_r->p != cpu.p) { \
  printf("difftest fail at " fmt ", expect %#x got %#x\n", ## __VA_ARGS__, ref_r->p, cpu.p); \
  return false; \
}

bool isa_difftest_checkregs(CPU_state *ref_r, vaddr_t pc) {
  int reg_num = ARRLEN(cpu.gpr);
  for (int i = 0; i < reg_num; i++) {
    CHECKDIFF_FMT(gpr[i], "gpr[%d]", i);
  }
  if (ref_r->pc != cpu.pc) {
    printf("difftest fail at pc, expect %#x got %#x\n", ref_r->pc, cpu.pc);
    return false;
  }
  CHECKDIFF_CSRS(mstatus);
  CHECKDIFF_CSRS(mtvec);
  CHECKDIFF_CSRS(mepc);
  CHECKDIFF_CSRS(mcause);
  return true;
}

void isa_difftest_attach() {}
