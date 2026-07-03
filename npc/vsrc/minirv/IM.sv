module IM(
  input  logic        rst_in,
  input  logic [31:0] pc_in,
  output logic [31:0] inst_out
);
  import "DPI-C" function int pmem_read(input int raddr);

  assign inst_out = rst_in ? 32'h0000_0013 : pmem_read(pc_in);
endmodule
