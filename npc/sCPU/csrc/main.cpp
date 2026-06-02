#include "VsCPU.h"
#include "verilated.h"
#include "verilated_fst_c.h"

static vluint64_t sim_time = 0;

double sc_time_stamp() {
  return sim_time;
}

static void step(VsCPU *dut, VerilatedFstC *tfp) {
  dut->clk = 0;
  dut->eval();
  tfp->dump(sim_time++);

  dut->clk = 1;
  dut->eval();
  tfp->dump(sim_time++);
}

int main(int argc, char **argv) {
  Verilated::commandArgs(argc, argv);
  Verilated::traceEverOn(true);

  auto *dut = new VsCPU;
  auto *tfp = new VerilatedFstC;

  dut->trace(tfp, 99);
  tfp->open("build/wave.fst");

  dut->rst = 1;
  step(dut, tfp);
  step(dut, tfp);

  dut->rst = 0;
  for (int cycle = 0; cycle < 128 && !Verilated::gotFinish(); cycle++) {
    step(dut, tfp);
  }

  VL_PRINTF("gpio = 0x%02x\n", dut->gpio);

  dut->final();
  tfp->close();
  delete tfp;
  delete dut;
  return 0;
}
