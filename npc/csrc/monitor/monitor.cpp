#include "monitor/monitor.h"

#include <cstdio>
#include <cstring>

#include <verilated.h>

#include "common.h"
#include "device/device.h"
#include "memory.h"
#include "monitor/sdb.h"
#include "dut.h"
#include "trace/trace.h"
#include "utils/log.h"
#include "utils/state.h"

int npc_monitor_main(int argc, char **argv) {
  const char *img_file = nullptr;
  const char *elf_file = nullptr;
  const char *log_file = nullptr;
  bool batch_mode = false;

  for (int i = 1; i < argc; ++i) {
    if (std::strcmp(argv[i], "-b") == 0) {
      batch_mode = true;
    } else if (std::strcmp(argv[i], "-e") == 0) {
      if (i + 1 >= argc) {
        std::fprintf(stderr, "Usage: %s [-b] [-l log] [-e elf] [image]\n", argv[0]);
        return 1;
      }
      elf_file = argv[++i];
    } else if (std::strcmp(argv[i], "-l") == 0) {
      if (i + 1 >= argc) {
        std::fprintf(stderr, "Usage: %s [-b] [-l log] [-e elf] [image]\n", argv[0]);
        return 1;
      }
      log_file = argv[++i];
    } else if (img_file == nullptr) {
      img_file = argv[i];
    } else {
      std::fprintf(stderr, "Usage: %s [-b] [-l log] [-e elf] [image]\n", argv[0]);
      return 1;
    }
  }

  Verilated::commandArgs(argc, argv);
  Verilated::traceEverOn(true);

  init_log(log_file);
  Log("Build time: %s, %s", __TIME__, __DATE__);
  Log("Trace: %s", "ON");
  Log("ITrace: %s", MUXDEF(CONFIG_ITRACE, "ON", "OFF"));
  Log("MTrace: %s", MUXDEF(CONFIG_MTRACE, "ON", "OFF"));
  Log("FTrace: %s", MUXDEF(CONFIG_FTRACE, "ON", "OFF"));
  Log("IRingBuf: %s", MUXDEF(CONFIG_IRINGBUF, "ON", "OFF"));
  Log("DiffTest: %s", MUXDEF(CONFIG_DIFFTEST, "ON", "OFF"));
  Log("Batch mode: %s", batch_mode ? "ON" : "OFF");
  if (img_file != nullptr) {
    Log("The image is %s", img_file);
  } else {
    Log("No image is given. Use the default built-in image.");
  }
  if (elf_file != nullptr) {
    Log("The ELF file is %s", elf_file);
  }

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
