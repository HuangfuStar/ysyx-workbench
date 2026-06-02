module instDec(
    input [7:0] inst,
    output [1:0] rs1,
    output [1:0] rs2,
    output [1:0] rd,
    output [1:0] opcode,
    output [3:0] addr,
    output [3:0] imm
);
    assign {opcode, rd, rs1, rs2} = inst;
    assign addr = inst[5:2];
    assign imm = inst[3:0];
endmodule
