module ALU(
  input  logic [31:0] a_in,
  input  logic [31:0] b_in,
  output logic [31:0] result_out
);
  assign result_out = a_in + b_in;
endmodule
