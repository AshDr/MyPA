#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "sdb.h"


#define NR_WP 32

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  /* TODO: Add more members if necessary */
  char* expr;
  uint32_t old_value;
} WP;

void init_wp_pool();
WP* new_wp();
void free_wp(int NO);
void wp_difftest();
void set_watch_point(char *expr, word_t val);
void iterate_wp();

#endif