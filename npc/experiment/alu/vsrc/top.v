module top(
  input  [7:0] sw,
  input  [4:0] btn,
  output [15:0] ledr,
  output [7:0] seg0
);
  wire [3:0] A;
  wire [3:0] B;
  wire [2:0] op;
  wire [3:0] C;
  wire OF;
  wire CF;
  wire ZF;

  assign A = sw[7:4];
  assign B = sw[3:0];
  assign op = btn[2:0];

  alu u_alu(
    .A(A),
    .B(B),
    .op(op),
    .C(C),
    .OF(OF),
    .CF(CF),
    .ZF(ZF)
  );

  seg7_hex u_seg(
    .x(C),
    .seg(seg0)
  );

  assign ledr[3:0] = C;
  assign ledr[4] = ZF;
  assign ledr[5] = OF;
  assign ledr[6] = CF;
  assign ledr[9:7] = op;
  assign ledr[11:10] = btn[4:3];
  assign ledr[15:12] = 4'b0;
endmodule
