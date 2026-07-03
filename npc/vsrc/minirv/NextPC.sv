module NextPC(
  input  logic [31:0] pc_in,
  input  logic        jmp_in,
  input  logic [31:0] next_pc_in,
  output logic [31:0] next_pc_out
);
  assign next_pc_out = jmp_in ? next_pc_in : (pc_in + 32'd4);
endmodule
