module mux2_4(
  input  [3:0] a,
  input  [3:0] b,
  input        sel,
  output [3:0] y
);
  assign y = sel ? b : a;
endmodule

module mux2_1(
  input  a,
  input  b,
  input  sel,
  output y
);
  assign y = sel ? b : a;
endmodule
