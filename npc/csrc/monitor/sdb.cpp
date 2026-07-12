#include "monitor/sdb.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <limits>
#include <string>

#include "common.h"
#include "monitor/expr.h"
#include "dut.h"
#include "trace/trace.h"
#include "utils/log.h"

namespace {

bool g_batch_mode = false;

int cmd_help(char *args);

char *trim_left(char *s) {
  if (s == nullptr) {
    return nullptr;
  }
  while (*s == ' ') {
    ++s;
  }
  return (*s == '\0') ? nullptr : s;
}

int cmd_c(char *args) {
  (void)args;
  sim_exec(std::numeric_limits<uint64_t>::max());
  return 0;
}

int cmd_q(char *args) {
  (void)args;
  return -1;
}

int cmd_si(char *args) {
  uint64_t n = 1;
  if (args != nullptr && std::sscanf(args, "%lu", &n) != 1) {
    std::puts("Usage: si [N]");
    return 0;
  }
  sim_exec(n);
  return 0;
}

int cmd_info(char *args) {
  if (args == nullptr) {
    std::puts("Usage: info r");
    return 0;
  }
  if (args[0] == 'r') {
    std::printf("pc = 0x%08x\n", sim_get_pc());
    for (int i = 0; i < sim_get_gpr_num(); ++i) {
      std::printf("%-4s x%-2d = 0x%08x\n", sim_get_gpr_name(i), i, sim_get_gpr(i));
    }
    return 0;
  }
  std::puts("Usage: info r");
  return 0;
}

int cmd_x(char *args) {
  if (args == nullptr) {
    std::puts("Usage: x N EXPR");
    return 0;
  }

  int n = 0;
  int expr_start = 0;
  if (std::sscanf(args, "%d%n", &n, &expr_start) != 1 || n <= 0) {
    std::puts("Usage: x N EXPR");
    return 0;
  }

  char *expr = args + expr_start;
  while (*expr == ' ') {
    ++expr;
  }
  if (*expr == '\0') {
    std::puts("Usage: x N EXPR");
    return 0;
  }

  bool success = false;
  uint32_t addr = expr_eval(expr, &success);
  if (!success) {
    std::puts("Bad expression");
    return 0;
  }

  for (int i = 0; i < n; ++i) {
    const uint32_t cur = addr + static_cast<uint32_t>(i * 4);
    std::printf("0x%08x: 0x%08x\n", cur, sim_pmem_read_word(cur));
  }
  return 0;
}

int cmd_p(char *args) {
  if (args == nullptr) {
    std::puts("Usage: p EXPR");
    return 0;
  }
  bool success = false;
  const uint32_t value = expr_eval(args, &success);
  if (!success) {
    std::puts("Bad expression");
    return 0;
  }
  std::printf("0x%08x (%u)\n", value, value);
  return 0;
}

int cmd_iringbuf(char *args) {
  (void)args; 
  trace_dump_iringbuf();
  return 0;
}

struct Command {
  const char *name;
  const char *description;
  int (*handler)(char *args);
};

Command kCmdTable[] = {
    {"help", "Display information about all supported commands", cmd_help},
    {"c", "Continue the execution of the program", cmd_c},
    {"q", "Exit NPC", cmd_q},
    {"si", "Single step execution", cmd_si},
    {"info", "Print register information", cmd_info},
    {"x", "Examine memory words", cmd_x},
    {"p", "Evaluate an expression", cmd_p},
    {"iringbuf", "Dump instruction ring buffer", cmd_iringbuf},
};

constexpr size_t kNrCmd = ARRLEN(kCmdTable);

int cmd_help(char *args) {
  if (args == nullptr) {
    for (size_t i = 0; i < kNrCmd; ++i) {
      std::printf("%s - %s\n", kCmdTable[i].name, kCmdTable[i].description);
    }
    return 0;
  }

  for (size_t i = 0; i < kNrCmd; ++i) {
    if (std::strcmp(args, kCmdTable[i].name) == 0) {
      std::printf("%s - %s\n", kCmdTable[i].name, kCmdTable[i].description);
      return 0;
    }
  }
  std::printf("Unknown command '%s'\n", args);
  return 0;
}

}  // namespace

void sdb_set_batch_mode() {
  g_batch_mode = true;
}

void init_sdb() {
}

void sdb_mainloop() {
  if (g_batch_mode) {
    cmd_c(nullptr);
    return;
  }

  std::string line;
  while (true) {
    std::cout << "(npc) " << std::flush;
    if (!std::getline(std::cin, line)) {
      return;
    }
    if (line.empty()) {
      continue;
    }

    char *command_line = &line[0];
    char *cmd = std::strtok(command_line, " ");
    if (cmd == nullptr) {
      continue;
    }
    char *args = command_line + std::strlen(cmd) + 1;
    args = trim_left(args);

    bool handled = false;
    for (size_t i = 0; i < kNrCmd; ++i) {
      if (std::strcmp(cmd, kCmdTable[i].name) == 0) {
        handled = true;
        if (kCmdTable[i].handler(args) < 0) {
          return;
        }
        break;
      }
    }
    if (!handled) {
      std::printf("Unknown command '%s'\n", cmd);
    }

    if (sim_is_halted()) {
      _Log("Program stopped at pc = 0x%08x, code = %d\n",
          static_cast<uint32_t>(sim_get_halt_pc()), sim_get_halt_code());
    }
  }
}
