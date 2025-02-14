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

#include "watchpoint.h"
#include "utils.h"

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
  }

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

//use Head Insert Method for free_ linked list
WP* new_wp() {
  if(free_ == NULL) {
    panic("No more free watchpoint!\n");
    return NULL;
  }
  WP *res = free_;
  free_ = free_->next;
  return res; 
}

void free_wp(int NO) {
  WP *cur = head, *prev = NULL;
  if(cur == NULL) {
    printf("No used wp rignt now.\n");
    return ;
  }
  if(cur->NO == NO) {
    head = head->next;
    cur->next = free_;
    return ;
  }
  while(cur != NULL) {
    if(cur->NO == NO) {
      prev->next = cur->next;
      cur->next = free_;
      return ;
    }
    prev = cur;
    cur = cur->next;
  }
  panic("Free watchpoint error!\n");
}

void set_watch_point(char *expr, word_t val) {
  WP *wp = new_wp();
  strcpy(wp->expr, expr);
  printf("-------------WPEXPR: %s\n", wp->expr);
  wp->old_value = val;
  if(head == NULL) {
    head = wp;
    head->next = NULL;
  }
  else {
    wp->next = head;
    head = wp;
  }
  return ;  
}

void iterate_wp() {
  WP *h = head;
  if(h == NULL) {
    printf(ANSI_FMT("NO watchpoint\n", ANSI_FG_BLUE));
    return ;
  }
  printf("%-8s%-8s%-8s\n", "Num", "Expr","Value");
  while (h) {
    printf("%-8d%-8s%-8u\n", h->NO, h->expr, h->old_value);
    h = h->next;
  }
}


void wp_difftest() {
  WP* h = head;
  while (h) {
    bool _;
    word_t new_val = expr(h->expr, &_);
    if (h->old_value != new_val) {
      printf("Watchpoint %d: %s\n"
        "Old value = %u\n"
        "New value = %u\n"
        , h->NO, h->expr, h->old_value, new_val);
          h->old_value = new_val;
        nemu_state.state = NEMU_STOP;
        return ;
    }
    h = h->next;
  }
}

