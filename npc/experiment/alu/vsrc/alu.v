module alu(
  input  [3:0] A,
  input  [3:0] B,
  input  [2:0] op,
  output [3:0] C,
  output       OF,
  output       CF,
  output       ZF
);
  wire is_add;
  wire is_sub;
  wire is_not;
  wire is_and;
  wire is_or;
  wire is_xor;
  wire is_slt;
  wire is_eq;

  wire [3:0] add_result;
  wire [3:0] sub_result;
  wire [3:0] not_result;
  wire [3:0] and_result;
  wire [3:0] or_result;
  wire [3:0] xor_result;
  wire [3:0] slt_result;
  wire [3:0] eq_result;
  wire add_cf;
  wire add_of;
  wire sub_cf;
  wire sub_of;
  wire signed_lt;
  wire equal_flag;

  alu_ctrl u_ctrl(
    .op(op),
    .is_add(is_add),
    .is_sub(is_sub),
    .is_not(is_not),
    .is_and(is_and),
    .is_or(is_or),
    .is_xor(is_xor),
    .is_slt(is_slt),
    .is_eq(is_eq)
  );

  adder4 u_add(
    .a(A),
    .b(B),
    .cin(1'b0),
    .s(add_result),
    .cout(add_cf),
    .of(add_of)
  );

  adder4 u_sub(
    .a(A),
    .b(~B),
    .cin(1'b1),
    .s(sub_result),
    .cout(sub_cf),
    .of(sub_of)
  );

  compare4 u_cmp(
    .a(A),
    .b(B),
    .sub_result(sub_result),
    .sub_of(sub_of),
    .lt(signed_lt),
    .eq(equal_flag)
  );

  assign not_result = ~A;
  assign and_result = A & B;
  assign or_result  = A | B;
  assign xor_result = A ^ B;
  assign slt_result = {3'b000, signed_lt};
  assign eq_result  = {3'b000, equal_flag};

  assign C =
      ({4{is_add}} & add_result) |
      ({4{is_sub}} & sub_result) |
      ({4{is_not}} & not_result) |
      ({4{is_and}} & and_result) |
      ({4{is_or }} & or_result ) |
      ({4{is_xor}} & xor_result) |
      ({4{is_slt}} & slt_result) |
      ({4{is_eq }} & eq_result );

  assign CF = (is_add & add_cf) | (is_sub & sub_cf);
  assign OF = (is_add & add_of) | (is_sub & sub_of);
  assign ZF = ~|C;
endmodule
