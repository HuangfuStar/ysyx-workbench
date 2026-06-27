module alu_ctrl(
  input  [2:0] op,
  output       is_add,
  output       is_sub,
  output       is_not,
  output       is_and,
  output       is_or,
  output       is_xor,
  output       is_slt,
  output       is_eq
);
  assign is_add = (op == 3'b000);
  assign is_sub = (op == 3'b001);
  assign is_not = (op == 3'b010);
  assign is_and = (op == 3'b011);
  assign is_or  = (op == 3'b100);
  assign is_xor = (op == 3'b101);
  assign is_slt = (op == 3'b110);
  assign is_eq  = (op == 3'b111);
endmodule
