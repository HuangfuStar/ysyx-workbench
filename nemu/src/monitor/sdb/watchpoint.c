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

#include "sdb.h"

#define NR_WP 32

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;
  char expr[256];
  word_t value;
} WP;

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

void new_wp(char *expr_str) {
  if (expr_str == NULL || *expr_str == '\0') {
    printf("Usage: w EXPR\n");
    return;
  }

  if (free_ == NULL) {
    printf("No free watchpoint\n");
    return;
  }

  if (strlen(expr_str) >= sizeof(free_->expr)) {
    printf("Watchpoint expression is too long\n");
    return;
  }

  bool success = false;
  word_t value = expr(expr_str, &success);
  if (!success) {
    printf("Bad expression\n");
    return;
  }

  WP *wp = free_;
  free_ = free_->next;
  wp->next = head;
  head = wp;

  strcpy(wp->expr, expr_str);
  wp->value = value;
  printf("Watchpoint %d: %s = " FMT_WORD "\n", wp->NO, wp->expr, wp->value);
}

void free_wp(int no) {
  WP *prev = NULL;
  WP *cur = head;
  while (cur != NULL && cur->NO != no) {
    prev = cur;
    cur = cur->next;
  }

  if (cur == NULL) {
    printf("No watchpoint %d\n", no);
    return;
  }

  if (prev == NULL) {
    head = cur->next;
  }
  else {
    prev->next = cur->next;
  }

  cur->next = free_;
  free_ = cur;
  printf("Delete watchpoint %d\n", no);
}

void display_wp() {
  if (head == NULL) {
    puts("No watchpoints");
    return;
  }

  printf("%-4s %-12s %s\n", "NO", "VALUE", "EXPR");
  for (WP *wp = head; wp != NULL; wp = wp->next) {
    printf("%-4d " FMT_WORD " %s\n", wp->NO, wp->value, wp->expr);
  }
}

bool check_wp() {
  bool stop = false;
  for (WP *wp = head; wp != NULL; wp = wp->next) {
    bool success = false;
    word_t new_value = expr(wp->expr, &success);
    if (!success) {
      printf("Watchpoint %d expression is invalid: %s\n", wp->NO, wp->expr);
      stop = true;
      continue;
    }

    if (new_value != wp->value) {
      printf("Watchpoint %d triggered\n", wp->NO);
      printf("expr: %s\n", wp->expr);
      printf("old value = " FMT_WORD "\n", wp->value);
      printf("new value = " FMT_WORD "\n", new_value);
      wp->value = new_value;
      stop = true;
    }
  }

  return stop;
}
