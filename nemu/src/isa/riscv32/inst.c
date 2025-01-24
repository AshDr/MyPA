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

#include "common.h"
#include "local-include/reg.h"
#include "macro.h"
#include <cpu/cpu.h>
#include <cpu/decode.h>
#include <cpu/ifetch.h>
#include <utils.h>

#define R(i) gpr(i)
#define Mr vaddr_read
#define Mw vaddr_write

enum {
  TYPE_I,
  TYPE_U,
  TYPE_S,
  TYPE_J,
  TYPE_N,
  TYPE_R,
  TYPE_B // none
};

#define src1R()                                                                \
  do {                                                                         \
    *src1 = R(rs1);                                                            \
  } while (0)
#define src2R()                                                                \
  do {                                                                         \
    *src2 = R(rs2);                                                            \
  } while (0)
#define immI()                                                                 \
  do {                                                                         \
    *imm = SEXT(BITS(i, 31, 20), 12);                                          \
  } while (0)
#define immU()                                                                 \
  do {                                                                         \
    *imm = SEXT(BITS(i, 31, 12), 20) << 12;                                    \
  } while (0)
#define immS()                                                                 \
  do {                                                                         \
    *imm = (SEXT(BITS(i, 31, 25), 7) << 5) | BITS(i, 11, 7);                   \
  } while (0)

#define immJ()                                                                 \
  do {                                                                         \
    *imm = SEXT(((BITS(i, 31, 31) << 19) | BITS(i, 30, 21) |                   \
                 (BITS(i, 20, 20) << 10) | (BITS(i, 19, 12) << 11))            \
                    << 1,                                                      \
                21);                                                           \
  } while (0)
// Log(ANSI_FG_CYAN "%#x\n" ANSI_NONE, *imm);
#define immB()                                                                 \
  do {                                                                         \
    *imm = (SEXT(BITS(i, 31, 31), 1) << 12) | (BITS(i, 7, 7) << 11) |          \
           (BITS(i, 30, 25) << 5) | (BITS(i, 11, 8) << 1);                     \
  } while (0)

static void decode_operand(Decode *s, int *rd, word_t *src1, word_t *src2,
                           word_t *imm, int type) {
  uint32_t i = s->isa.inst.val;
  int rs1 = BITS(i, 19, 15);
  int rs2 = BITS(i, 24, 20);
  *rd = BITS(i, 11, 7);
  // for(int o = sizeof(uint32_t) * 8 - 1; o >= 0; --o) {
  //     printf("%d", (i >> o) & 1); // 逐位打印
  // }
  // puts("");
  // printf("!!!\n");
  // printf("rs1: %u, rs2: %u, rd: %u , type: %d \n", rs1, rs2, *rd, type);
  switch (type) {
  case TYPE_I:
    src1R();
    immI();
    break;
  case TYPE_U:
    immU();
    break;
  case TYPE_S:
    src1R();
    src2R();
    immS();
    break;
  case TYPE_J:
    immJ();
    break;
  case TYPE_B:
    src1R();
    src2R();
    immB();
    break;
  case TYPE_R:
    src1R();
    src2R();
    break;
  }
  // Log("Address: %x, src1: %u, src2: %u, imm:%u\n", s->pc, *src1, *src2,
  // *imm); if(type == TYPE_B) {
  //   Log(ANSI_FMT("This B type command jump to: %x\n", ANSI_FG_RED), s->pc +
  //   *imm);
  // }
}

static vaddr_t *csr_register(word_t imm) {
  switch (imm) {
  case 0x341:
    return &(cpu.csrs.mepc);
  case 0x342:
    return &(cpu.csrs.mcause);
  case 0x300:
    return &(cpu.csrs.mstatus);
  case 0x305:
    return &(cpu.csrs.mtvec);
  default:
    panic("Unknown csr");
  }
}

#define ECALL(dnpc)                                                            \
  {                                                                            \
    bool success;                                                              \
    dnpc = (isa_raise_intr(isa_reg_str2val("a7", &success), s->pc));           \
  }
#define CSR(i) *csr_register(i)

static int decode_exec(Decode *s) {
  int rd = 0;
  word_t src1 = 0, src2 = 0, imm = 0;
  s->dnpc = s->snpc;
#define INSTPAT_INST(s) ((s)->isa.inst.val)
#define INSTPAT_MATCH(s, name, type, ... /* execute body */)                   \
  {                                                                            \
    decode_operand(s, &rd, &src1, &src2, &imm, concat(TYPE_, type));           \
    __VA_ARGS__;                                                               \
  }
  //   printf("%u!", INSTPAT_INST(s));

  INSTPAT_START();
  // lb
  INSTPAT("??????? ????? ????? 000 ????? 00000 11", lb, I,
          R(rd) = SEXT(Mr(src1 + imm, 1), 8));
  INSTPAT("??????? ????? ????? 010 ????? 00000 11", lw, I,
          R(rd) = SEXT(Mr(src1 + imm, 4), 32));
  // lh signed?
  INSTPAT("??????? ????? ????? 001 ????? 00000 11", lh, I,
          R(rd) = SEXT(Mr(src1 + imm, 2), 16));

  INSTPAT("??????? ????? ????? 000 ????? 11001 11", jalr, I,
          s->dnpc = (src1 + imm) & ~(word_t)1;
          R(rd) = s->snpc; IFDEF(CONFIG_FTRACE, {
            if (rd == 1) {
              // func call use x1(ra)
              trace_func_call(s->pc, s->dnpc);
            } else if (rd == 0) {
              // func return use x0(zero)
              trace_func_ret(s->pc);
            }
            // other case doesn't print
          }));

  INSTPAT("??????? ????? ????? 010 ????? 01000 11", sw, S,
          Mw(src1 + imm, 4, src2));

  INSTPAT("??????? ????? ????? ??? ????? 11011 11", jal, J,
          s->dnpc = s->pc + imm;
          R(rd) = s->snpc; IFDEF(CONFIG_FTRACE, {
            if (rd == 1) {
              // func call use x1(ra)
              trace_func_call(s->pc, s->dnpc);
            }
          }));

  INSTPAT("??????? ????? ????? ??? ????? 00101 11", auipc, U,
          R(rd) = s->pc + imm);
  INSTPAT("??????? ????? ????? 100 ????? 00000 11", lbu, I,
          R(rd) = Mr(src1 + imm, 1));
  INSTPAT("??????? ????? ????? 101 ????? 00000 11", lhu, I,
          R(rd) = Mr(src1 + imm, 2));
  INSTPAT("??????? ????? ????? 000 ????? 01000 11", sb, S,
          Mw(src1 + imm, 1, src2));
  INSTPAT("??????? ????? ????? 000 ????? 00100 11", addi, I,
          R(rd) = src1 + imm);
  INSTPAT("0000000 00001 00000 000 00000 11100 11", ebreak, N,
          NEMUTRAP(s->pc, R(10))); // R(10) is $a0
  INSTPAT("??????? ????? ????? ??? ????? 01101 11", lui, U, R(rd) = imm);
  INSTPAT("0000000 ????? ????? 000 ????? 01100 11", add, R,
          R(rd) = src1 + src2);
  INSTPAT("??????? ????? ????? 001 ????? 11000 11", bne, B,
          s->dnpc = (src1 != src2) ? s->pc + imm : s->dnpc);
  INSTPAT("0000000 ????? ????? 010 ????? 01100 11", slt, R,
          R(rd) = ((sword_t)src1 < (sword_t)src2) ? 1 : 0);
  INSTPAT("0000000 ????? ????? 011 ????? 01100 11", sltu, R,
          R(rd) = (src1 < src2) ? 1 : 0);
  INSTPAT("??????? ????? ????? 010 ????? 00100 11", slti, I,
          R(rd) = ((sword_t)src1 < (sword_t)imm) ? 1 : 0);
  INSTPAT("??????? ????? ????? 011 ????? 00100 11", sltiu, I,
          R(rd) = (src1 < imm) ? 1 : 0);
  INSTPAT("??????? ????? ????? 000 ????? 11000 11", beq, B,
          s->dnpc = (src1 == src2) ? s->pc + imm : s->dnpc);
  INSTPAT("0100000 ????? ????? 000 ????? 01100 11", sub, R,
          R(rd) = src1 - src2);
  INSTPAT("0000000 ????? ????? 100 ????? 01100 11", xor, R,
          R(rd) = src1 ^ src2);
  INSTPAT("0000000 ????? ????? 110 ????? 01100 11", or, R, R(rd) = src1 | src2);
  INSTPAT("0000000 ????? ????? 111 ????? 01100 11", and, R,
          R(rd) = src1 & src2);
  INSTPAT("??????? ????? ????? 001 ????? 01000 11", sh, S,
          Mw(src1 + imm, 2, src2));
  INSTPAT("0100000 ????? ????? 101 ????? 00100 11", srai, I,
          R(rd) = (sword_t)src1 >> BITS(imm, 4, 0));
  INSTPAT("??????? ????? ????? 111 ????? 00100 11", andi, I,
          R(rd) = src1 & imm);
  INSTPAT("??????? ????? ????? 110 ????? 00100 11", ori, I, R(rd) = src1 | imm);
  INSTPAT("??????? ????? ????? 100 ????? 00100 11", xori, I,
          R(rd) = src1 ^ imm);
  INSTPAT("0000000 ????? ????? 001 ????? 01100 11", sll, R,
          R(rd) = src1 << BITS(src2, 4, 0));
  INSTPAT("0000000 ????? ????? 101 ????? 01100 11", srl, R,
          R(rd) = src1 >> BITS(src2, 4, 0));
  INSTPAT("0000000 ????? ????? 101 ????? 00100 11", srli, I,
          R(rd) = src1 >> BITS(imm, 4, 0));
  INSTPAT("0000000 ????? ????? 001 ????? 00100 11", slli, I,
          R(rd) = src1 << BITS(imm, 4, 0));
  INSTPAT("??????? ????? ????? 111 ????? 11000 11", bgeu, B,
          s->dnpc = (src1 >= src2) ? s->pc + imm : s->dnpc);
  INSTPAT("??????? ????? ????? 110 ????? 11000 11", bltu, B,
          s->dnpc = (src1 < src2) ? s->pc + imm : s->dnpc);
  INSTPAT("??????? ????? ????? 100 ????? 11000 11", blt, B,
          s->dnpc = ((sword_t)src1 < (sword_t)src2) ? s->pc + imm : s->dnpc);
  INSTPAT("??????? ????? ????? 101 ????? 11000 11", bge, B,
          s->dnpc = ((sword_t)src1 >= (sword_t)src2) ? s->pc + imm : s->dnpc);
  INSTPAT("0000001 ????? ????? 000 ????? 01100 11", mul, R,
          R(rd) = (sword_t)src1 * (sword_t)src2);
  INSTPAT("0000001 ????? ????? 100 ????? 01100 11", div, R,
          R(rd) = (sword_t)src1 / (sword_t)src2);
  INSTPAT("0000001 ????? ????? 110 ????? 01100 11", rem, R,
          R(rd) = (sword_t)src1 % (sword_t)src2);
  INSTPAT("0000001 ????? ????? 001 ????? 01100 11", mulh, R,
          R(rd) = ((SEXT(src1, 32) * SEXT(src2, 32)) >> 32));
  // mulhu
  INSTPAT("0000001 ????? ????? 011 ????? 01100 11", mulhu, R,
          R(rd) = ((uint64_t)src1 * (uint64_t)src2) >> 32);
  INSTPAT("0000001 ????? ????? 111 ????? 01100 11", remu, R,
          R(rd) = src1 % src2);
  INSTPAT("0000001 ????? ????? 101 ????? 01100 11", divu, R,
          R(rd) = src1 / src2);
  INSTPAT("0100000 ????? ????? 101 ????? 01100 11", sra, R,
          R(rd) = (sword_t)src1 >> BITS(src2, 4, 0));
  INSTPAT("??????? ????? ????? 001 ????? 11100 11", csrrw, I, R(rd) = CSR(imm);
          CSR(imm) = src1);
  INSTPAT("??????? ????? ????? 010 ????? 11100 11", csrrs, I, R(rd) = CSR(imm);
          CSR(imm) |= src1);
  INSTPAT("0000000 00000 00000 000 00000 11100 11", ecall, I, ECALL(s->dnpc));
  // mret
  INSTPAT("0011000 00010 00000 000 00000 11100 11", mret, N, {
    s->dnpc = CSR(0x341);
    cpu.csrs.mstatus = 0x80; // MPIE
    R(0) = 0;
  });
  INSTPAT("??????? ????? ????? ??? ????? ????? ??", inv, N, INV(s->pc));
  INSTPAT_END();

  R(0) = 0; // reset $zero to 0
  // beq bne imm+pc or pc = imm
  return 0;
}

int isa_exec_once(Decode *s) {
  s->isa.inst.val = inst_fetch(&s->snpc, 4);
  return decode_exec(s);
}
