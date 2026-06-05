module IM(
  input  logic [31:0] pc_in,
  output logic [31:0] inst_out
);
  import "DPI-C" function int pmem_read(input int raddr);

  assign inst_out = pmem_read(pc_in);
endmodule
