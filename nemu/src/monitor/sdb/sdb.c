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

#include <isa.h>
#include <cpu/cpu.h>
#include <memory/vaddr.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "sdb.h"

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

static int cmd_si(char *args) {
    uint64_t n;
    if (NULL == args) n = 1;
    else if (sscanf(args, "%lu", &n) != 1) 
        puts("Usage: si [<num>]");
    cpu_exec(n);
    return 0;
}

static int cmd_info(char *args) {
    if (args == NULL) {
        printf("usages:\n"
               "    info r: print the registers\n"
               "    info w: print the watchpoints\n");
        return 0;
    }
    if ('r' == *args)
        isa_reg_display();
    if ('w' == *args) 
        display_wp();
    return 0;
}

static int cmd_w(char *args) {
    if (args == NULL) {
        puts("Usage: w EXPR");
        return 0;
    }
    new_wp(args);
    return 0;
}

static int cmd_d(char *args) {
    int no = -1;
    if (args == NULL || sscanf(args, "%d", &no) != 1) {
        puts("Usage: d N");
        return 0;
    }
    free_wp(no);
    return 0;
}

static int cmd_x(char *args) {
    const char *usage = 
             "Usages:\n"
             "    x N Expr: evaluate Expr as address, print next N 4-byte words in memory";
    if (NULL == args) {
        puts("x should need arguments");
        puts(usage);
        return 0;
    }

    int n;
    int expr_start = 0;
    if (sscanf(args, "%d%n", &n, &expr_start) != 1 || n <= 0) {
        puts("Invalid arguments");
        puts(usage);
        return 0;
    }

    char *e = args + expr_start;
    while (*e == ' ') {
        e ++;
    }
    if (*e == '\0') {
        puts("Missing expression");
        puts(usage);
        return 0;
    }

    bool success;
    vaddr_t vaddr = expr(e, &success);
    if (!success) {
        puts("Bad expression");
        return 0;
    }

    const int columns = 8;
    for (int i = 0; i < n; i++) {
        vaddr_t cur = vaddr + i * 4;
        word_t word = vaddr_read(cur, 4);

        if (i % columns == 0) {
            printf(FMT_WORD ":", cur);
        }
        printf(" %08x", (unsigned)(word & 0xffffffffu));
        if (i % columns == columns - 1 || i == n - 1) {
            puts("");
        }
    }

    return 0;
}

static int cmd_q(char *args) {
  return -1;
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
  /* TODO: Add more commands */
  { "si", "Single instruction", cmd_si },
  { "info", "Print the register or watchpoint info", cmd_info },
  { "x", "Read bytes from the memory", cmd_x },
  { "w", "Set a watchpoint", cmd_w },
  { "d", "Delete a watchpoint", cmd_d },

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

void sdb_mainloop() {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

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
