#ifndef NPC_UTILS_LOG_H
#define NPC_UTILS_LOG_H

#include <stdbool.h>
#include <stdio.h>

#define ANSI_FG_BLUE "\33[1;34m"
#define ANSI_NONE "\33[0m"
#define ANSI_FMT(str, fmt) fmt str ANSI_NONE

#ifdef __cplusplus
extern "C" {
#endif

extern FILE *log_fp;

void init_log(const char *log_file);
bool log_enable(void);

#ifdef __cplusplus
}
#endif

#define log_write(...) \
  do { \
    if (log_enable() && log_fp != NULL) { \
      fprintf(log_fp, __VA_ARGS__); \
      fflush(log_fp); \
    } \
  } while (0)

#define _Log(...) \
  do { \
    printf(__VA_ARGS__); \
    log_write(__VA_ARGS__); \
  } while (0)

#define Log(format, ...) \
  _Log(ANSI_FMT("[%s:%d %s] " format, ANSI_FG_BLUE) "\n", \
      __FILE__, __LINE__, __func__, ##__VA_ARGS__)

#endif
