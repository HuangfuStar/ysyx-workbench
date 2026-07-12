#include "utils/log.h"

FILE *log_fp = nullptr;

void init_log(const char *log_file) {
  log_fp = stdout;
  if (log_file != nullptr) {
    FILE *fp = fopen(log_file, "w");
    if (fp == nullptr) {
      fprintf(stderr, "Can not open '%s'\n", log_file);
      return;
    }
    log_fp = fp;
  }
  Log("Log is written to %s", log_file != nullptr ? log_file : "stdout");
}

bool log_enable(void) {
  return true;
}
