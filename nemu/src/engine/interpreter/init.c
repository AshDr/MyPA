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

#include "debug.h"
#include <cpu/cpu.h>

void sdb_mainloop();
char buffer[65536];
void engine_start() {
#ifdef CONFIG_TARGET_AM
  cpu_exec(-1);
#else
  /* Receive commands from user. */
  FILE *file = fopen("/home/ashdr/code/ics2023/nemu/tools/gen-expr/build/input", "r");
  if(file == NULL) {
    Log("Open expr test file failed!\n");
  }else {
    while(fgets(buffer, sizeof(buffer), file) != NULL) {
      printf("%s", buffer);
      char *args = strtok(buffer, " ");
      if(args == NULL) continue;
      printf("arg1: %s", args);
      args = strtok(NULL, " ");
      printf("arg2: %s", args);
    }
    fclose(file);
  }
  sdb_mainloop();
#endif
}
