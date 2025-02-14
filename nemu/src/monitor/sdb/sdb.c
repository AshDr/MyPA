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

#include <ctype.h>
#include <isa.h>
#include <cpu/cpu.h>
#include <readline/chardefs.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "sdb.h"
#include "watchpoint.h"
#include <stdint.h>
#include <utils.h>
#include <memory/vaddr.h>

static int is_batch_mode = false;

void init_regex();
void init_wp_pool();

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}


static int cmd_q(char *args) {
  nemu_state.state = NEMU_QUIT;
  return -1;
}

int cnt = 0;
static int cmd_p(char *args) {
  bool flag = true;
  word_t val = expr(args,&flag);
  if(args == NULL) {
    printf("Args is NULL\n");
    return 0;
  }
  printf("args: %s\n", args);
  if(!flag) {
    ++cnt;
    printf(ANSI_FMT("Runtime Error during evalute expression\n", ANSI_FG_RED));
  }else {
    printf(ANSI_FMT("Expression result: %u\n", ANSI_FG_GREEN), val);
  }
  return 0;
}

static int cmd_si(char *args) {
  if(args == NULL) {
    cpu_exec(1);
  }
  else {
    char *arg = strtok(args, " ");
    uint64_t sum = 0;
    for(int i = 0; i < strlen(arg); i++) {
      if(!isdigit(arg[i])) {
        printf("Paramater should be digit!\n");
        return 0;
      }
      sum = sum * 10 + arg[i] - '0';
    }
    cpu_exec(sum);
  }
  return 0;
}

static int cmd_info(char *args) {
  char *arg = strtok(NULL, " ");
  if (arg == NULL) {
    printf("Usage: info r (registers) or info w (watchpoints)\n");
  } else {
    if (strcmp(arg, "r") == 0) {
      isa_reg_display();
    } else if (strcmp(arg, "w") == 0) {
      // todo
      iterate_wp();
    } else {
      printf("Usage: info r (registers) or info w (watchpoints)\n");
    }
  }
  return 0;
}
static int cmd_x(char *args) {
  char *arg1 = strtok(NULL, " ");
  if (arg1 == NULL) {
    printf("Usage: x N EXPR\n");
    return 0;
  }
  char *arg2 = strtok(NULL, " ");
  if (arg2 == NULL) {
    printf("Usage: x N EXPR\n");
    return 0;
  }

  int n = strtol(arg1, NULL, 10);
  vaddr_t expr = strtol(arg2, NULL, 16);

  int i, j;
  for (i = 0; i < n;) {
    printf(ANSI_FMT("%#018x: ", ANSI_FG_CYAN), expr);
    
    for (j = 0; i < n && j < 4; i++, j++) {
      word_t w = vaddr_read(expr, 4);
      expr += 4;
      printf("%#018x ", w);
    }
    puts("");
  }
  
  return 0;
}

static int cmd_w(char *args) {
  if(args == NULL) {
    printf("Need expression!\n");
    return 0;
  }
  bool flag = true;
  word_t val = expr(args, &flag);
  if(flag == false) {
    printf(ANSI_FMT("Invalid expression for watchpoint!\n", ANSI_FG_RED));
  }else {
    set_watch_point(args, val);

  }
  return 0;
}

static int cmd_d(char *args) {
  int no = strtol(args, NULL, 10);
  free_wp(no);
  return 0;
}

static int cmd_help(char *args);

static struct {
  const char *name;
  const char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display information about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  {"si", "Single Step", cmd_si},
  {"info", "Show information", cmd_info},
  {"x", "Usage: x N EXPR. Scan the memory from EXPR by N bytes", cmd_x},
  {"p", "Usage: p EXPR. Calculate the expression, e.g. p $eax + 1", cmd_p },
  {"w", "Usage: w EXPR. Add watchpoint on input expression.",cmd_w},
  {"d", "Usage: Delete watchpoint.", cmd_d}
  /* TODO: Add more commands */
};

#define NR_CMD ARRLEN(cmd_table)

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void sdb_set_batch_mode() {
  is_batch_mode = true;
}
char buffer[65536];
void sdb_mainloop() {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }
  /* Add test case here*/
  // FILE *file = fopen("/home/ashdr/code/ics2023/nemu/tools/gen-expr/build/input", "r");
  // if(file == NULL) {
  //   Log("Open expr test file failed!\n");
  // }else {
  //   while(fgets(buffer, sizeof(buffer), file) != NULL) {
  //     buffer[strcspn(buffer, "\n")] = '\0';
  //     printf("%s\n", buffer);
  //     char *args = strtok(buffer, " ");
  //     if(args == NULL) continue;
  //     args = strtok(NULL, " ");
  //     cmd_p(args);
  //   }
  //   fclose(file);
  // }
  // printf("Wrong expr cnt: %d\n", cnt);



  for (char *str; (str = rl_gets()) != NULL; ) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}

void init_sdb() {
  /* Compile the regular expressions. */
  init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();
}
