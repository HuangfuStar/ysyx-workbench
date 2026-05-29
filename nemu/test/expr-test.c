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

#include <common.h>
#include "../src/monitor/sdb/sdb.h"

void init_regex();

int run_expr_test(const char *input_file) {
  FILE *fp = fopen(input_file, "r");
  if (fp == NULL) {
    printf("Can not open '%s'\n", input_file);
    return 1;
  }

  init_regex();

  char line[65536 + 128];
  char expr_buf[65536];
  int nr_test = 0;
  while (fgets(line, sizeof(line), fp) != NULL) {
    uint32_t expected = 0;
    if (sscanf(line, "%u %65535[^\n]", &expected, expr_buf) != 2) {
      printf("bad input line: %s", line);
      fclose(fp);
      return 1;
    }

    bool success = false;
    word_t actual = expr(expr_buf, &success);
    if (!success || (uint32_t)actual != expected) {
      printf("test failed at line %d\n", nr_test + 1);
      printf("expr: %s\n", expr_buf);
      printf("expected: %u, actual: %u\n", expected, (uint32_t)actual);
      fclose(fp);
      return 1;
    }
    nr_test ++;
  }

  fclose(fp);
  printf("PASS: %d expressions\n", nr_test);
  return 0;
}
