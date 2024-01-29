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
  TK_LBR,TK_RBR,TK_REG,TK_VAR,TK_SIGN
};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */
  {"\\(uint32_t\\)",TK_SIGN},
  {" +", TK_NOTYPE},    // spaces
  {"\\+", TK_PLUS},         // plus
  {"==", TK_EQ},        // equal
  {"-",TK_SUB},
  {"\\*",TK_MUL},
  {"\\/",TK_DIV},
  {"[0-9]+",TK_NUM},
  {"\\(", TK_LBR},
  {"\\)", TK_RBR},
  {"\\$\\w+", TK_REG},
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
      printf("%c??\n", e[position]);
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

bool check_parentheses(int p, int q) {
  if(tokens[p].type == TK_LBR && tokens[q].type == TK_RBR) {
    int sum = 0;
    for(int i = p; i < q; i++) {
      if(tokens[i].type == TK_LBR) ++sum;
      else if(tokens[i].type == TK_RBR) --sum;
      if(sum < 0) return false;
    }
    return sum == 0;    
  }
  return false;
}
bool is_arithmetic(int tp) {
  if(tp == TK_EQ || tp == TK_NUM || tp == TK_LBR || tp == TK_RBR) return false;
  return true;
}
int get_level(int tp) {
  if(tp == TK_PLUS || tp == TK_SUB) return 1;
  if(tp == TK_MUL || tp == TK_DIV) return 2;
  return 3;
}
int find_major(int p, int q) { // can not be () form
  int pos = -1,prelv = 100;
  for(int i = p; i <= q; i++) {
    if(is_arithmetic(tokens[i].type)) {
      if(get_level(tokens[i].type) < prelv) {
        prelv = get_level(tokens[i].type);
        pos = i;
      }
    }
  }
  Assert(pos != -1, "Major pos find error! At range (%d, %d)", p, q);
  return pos;
}
word_t eval(int p, int q, bool *ok) {
  // *ok = true;
  if(p > q) {
    *ok = false;
    return 0;
  }
  else if(p == q) {
    if(tokens[p].type != TK_NUM) {
      *ok = false;
      return 0;
    }
    word_t res = strtol(tokens[p].str, NULL, 10);
    return res;
  }else if(check_parentheses(p, q) == true) {
    return eval(p + 1, q - 1, ok);
  }else {
    int mpos = find_major(p, q);
    word_t val1 = eval(p, mpos - 1, ok),val2 = eval(mpos + 1, q, ok);
    int op_tp = tokens[mpos].type;
    switch (op_tp) {
      case TK_PLUS: {
        return val1 + val2;
      }
      case TK_SUB: {
        return val1 - val2;
      }
      case TK_MUL: {
        return val1 * val2;
      }
      case TK_DIV: {
        //divided by zero ?
        if(val2 == 0) {
          *ok = false;
          return 0;
        }
        return (sword_t)val1 / (sword_t)val2; // e.g (1 - 2) / 2 should be 0 not 2147483647, because we generate test case and the result of test case if 0
      }
      default: panic("Wrong major operator at range(%d, %d), mpos is %d", p, q, mpos);
    }
  }
}
word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }
  /* TODO: Insert codes to evaluate the expression. */
  // TODO();
  return eval(0, nr_token - 1, success);;
}
