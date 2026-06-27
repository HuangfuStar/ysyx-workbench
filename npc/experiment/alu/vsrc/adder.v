module adder4(
  input  [3:0] a,
  input  [3:0] b,
  input        cin,
  output [3:0] s,
  output       cout,
  output       of
);
  wire [4:0] sum_ext;

  assign sum_ext = {1'b0, a} + {1'b0, b} + cin;
  assign s = sum_ext[3:0];
  assign cout = sum_ext[4];
  assign of = ~(a[3] ^ b[3]) & (a[3] ^ s[3]);
endmodule
