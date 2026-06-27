module compare4(
  input  [3:0] a,
  input  [3:0] b,
  input  [3:0] sub_result,
  input        sub_of,
  output       lt,
  output       eq
);
  assign lt = sub_result[3] ^ sub_of;
  assign eq = ~|(a ^ b);
endmodule
