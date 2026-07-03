#include "monitor/monitor.h"

#include <cstdio>
#include <cstring>

#include <verilated.h>

#include "device/device.h"
#include "memory.h"
#include "monitor/sdb.h"
#include "dut.h"
#include "trace/trace.h"
#include "utils/state.h"

int npc_monitor_main(int argc, char **argv) {
  const char *img_file = nullptr;
  const char *elf_file = nullptr;
  bool batch_mode = false;

  for (int i = 1; i < argc; ++i) {
    if (std::strcmp(argv[i], "-b") == 0) {
      batch_mode = true;
    } else if (std::strcmp(argv[i], "-e") == 0) {
      if (i + 1 >= argc) {
        std::fprintf(stderr, "Usage: %s [-b] [-e elf] [image]\n", argv[0]);
        return 1;
      }
      elf_file = argv[++i];
    } else if (img_file == nullptr) {
      img_file = argv[i];
    } else {
      std::fprintf(stderr, "Usage: %s [-b] [-e elf] [image]\n", argv[0]);
      return 1;
    }
  }

  Verilated::commandArgs(argc, argv);
  Verilated::traceEverOn(true);

  npc_init_state();
  
  init_device();

  load_image(img_file);

  init_sdb();
  trace_init(elf_file);

  if (batch_mode) {
    sdb_set_batch_mode();
  }

  sim_init_dut("build/wave.fst");

  npc_start_timer();

  sdb_mainloop();

  sim_finish_dut();

  return npc_report_exit_status();
}
