/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
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

#include "common.h"
#include "memory/vaddr.h"
#include <isa.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>
#include <stdlib.h>
#include <string.h>

enum {
  TK_NOTYPE = 256, TK_EQ,
  /* TODO: Add more token types */
  TK_SUB,TK_PLUS,TK_MUL,TK_DIV,TK_NUM,
  TK_LBR,TK_RBR,TK_REG,TK_VAR,TK_SIGN,
  TK_AND,TK_NEQ,TK_DREF,TK_HEX
};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */
  {"\\(uint32_t\\)",TK_SIGN},
  {"0[xX][0-9A-Fa-f]+",TK_HEX},
  {" +", TK_NOTYPE},    // spaces
  {"\\+", TK_PLUS},         // plus
  {"==", TK_EQ},        // equal
  {"-",TK_SUB},
  {"\\*",TK_MUL},
  {"\\/",TK_DIV},
  {"[0-9]+",TK_NUM},
  {"\\(", TK_LBR},
  {"\\)", TK_RBR},
  {"\\$[$]?\\w+", TK_REG},
  {"[A-Za-z_]\\w*", TK_VAR},
  
};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;
static Token tokens[65536] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;
        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */
        if(rules[i].token_type == TK_NOTYPE || rules[i].token_type == TK_SIGN) break;
        tokens[nr_token].type = rules[i].token_type;
        switch (rules[i].token_type) {
          default: {
            //Overflow when substr_len > 32
            Assert(substr_len < 32, "Length of each token should be smaller or equal than 32");
            strncpy(tokens[nr_token].str, substr_start, substr_len);
            tokens[nr_token].str[substr_len] = '\0';
          }
        }
        ++nr_token;
        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

bool check_parentheses(int p, int q) {
  if(tokens[p].type == TK_LBR && tokens[q].type == TK_RBR) {
    int sum = 0, cnt = 0;
    for(int i = p; i <= q; i++) {
      if(tokens[i].type == TK_LBR) ++sum;
      else if(tokens[i].type == TK_RBR) {
        --sum;
        if(sum == 0) ++cnt;
      }
      if(sum < 0) return false;
    }
    return sum == 0 && cnt == 1;    
  }
  return false;
}
bool is_arithmetic(int tp) {
  if(tp == TK_NUM || tp == TK_LBR || tp == TK_RBR || tp == TK_HEX) return false;
  return true;
}

int get_level(int tp) {
  if(tp == TK_PLUS || tp == TK_SUB) return 1;
  if(tp == TK_MUL || tp == TK_DIV) return 2;
  return 3; // TK_DREF
}

int find_major(int p, int q) { // can not be () form
  int pos = -1,prelv = 100;
  int pnum = 0;
  for(int i = p; i <= q; i++) {
    if(tokens[i].type == TK_LBR) ++pnum;
    else if(tokens[i].type == TK_RBR) --pnum;
    else if(is_arithmetic(tokens[i].type)) {
      if(pnum == 0 && get_level(tokens[i].type) <= prelv) {
        prelv = get_level(tokens[i].type);
        pos = i;
      }
    }
  }
  if(pos == -1) {
    for(int i = p; i <= q; i++) {
      printf("%s  ", tokens[i].str);
    }
    Assert(pos != -1, "Major pos find error! At range (%d, %d)", p, q);
  }
  return pos;
}

word_t eval(int p, int q, bool *ok) {
  // *ok = true;
  printf("(%d %d)\n", p, q);
  if(p > q) {
    printf(ANSI_FMT("Range error! (%d, %d)\n", ANSI_FG_RED), p, q);
    *ok = false;
    return 0;
  }
  else if(p == q) {
    if(tokens[p].type == TK_NUM) {
      word_t res = strtol(tokens[p].str, NULL, 10);
      return res;
    }
    else if(tokens[p].type == TK_HEX) {
      word_t res = strtol(tokens[p].str, NULL, 16);
      return res;
    }
    else if(tokens[p].type == TK_REG) {
        bool flag = true;
        word_t res = isa_reg_str2val(tokens[p].str, &flag);
        if(flag == false) {
          printf(ANSI_FMT("Reg error!\n", ANSI_FG_RED));
          *ok = false;
          return 0;
        }
        return res;
    }
    else {
      printf(ANSI_FMT("Type error!\n", ANSI_FG_RED));
      *ok = false;
      return 0;
    }
    
  }else if(check_parentheses(p, q) == true) {
    // Log("check_parentheses(%d %d) is ok\n", p, q);
    return eval(p + 1, q - 1, ok);
  }else {
    int mpos = find_major(p, q);
    int op_tp = tokens[mpos].type;
    word_t val1 = 0, val2 = 0; 
    if(op_tp == TK_DREF) {
      val2 = eval(mpos + 1, q, ok);
    }else {
      val1= eval(p, mpos - 1, ok),val2 = eval(mpos + 1, q, ok);
    }
    
    switch (op_tp) {
      case TK_PLUS: {
        return val1 + val2;
      }break;
      case TK_SUB: {
        return val1 - val2;
      }break;
      case TK_MUL: {
        return val1 * val2;
      }break;
      case TK_DIV: {
        //divided by zero ?
        if(val2 == 0) {
          printf(ANSI_FMT("Divided by zero!(%d,%d)\n", ANSI_FG_RED),p, q);
          *ok = false;
          return 0;
        }
        return val1 / val2; // e.g (1 - 2) / 2 should be 0 not 2147483647, because we generate test case and the result of test case if 0
      }break;
      case TK_DREF:{
          return vaddr_read(val2, 4); //get uint32_t from ram
      }
      default: panic("Wrong major operator at range(%d, %d), mpos is %d", p, q, mpos);
    }
  }
}
word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    printf(ANSI_FMT("Make token faied!\n", ANSI_FG_RED));
    return 0;
  }
  for (int i = 0; i < nr_token; i ++) {
    if (tokens[i].type == TK_MUL && (i == 0 || is_arithmetic(tokens[i - 1].type))) {
      tokens[i].type = TK_DREF;
    }
  }
  // return 0;
  printf("Expr Tokens: ");
  for(int i = 0; i < nr_token; i++) {
    printf("%s",tokens[i].str);
  }
  puts("");
  /* TODO: Insert codes to evaluate the expression. */
  return eval(0, nr_token - 1, success);
}