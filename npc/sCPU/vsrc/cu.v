module cu(
    input [1:0] op,
    output RegWE,
    output RegSource,
    output JMP,
    output SubCtr,
    output imm2Reg,
    output gpioWE
);
    assign RegWE = (~op[1] & ~op[0]) | (op[1] & ~op[0]);
    assign RegSource = op[1] & ~op[0];
    assign JMP = op[1] & op[0];
    assign SubCtr = 1'b0;
    assign imm2Reg = op[1] & ~op[0];
    assign gpioWE = ~op[1] & op[0];
endmodule
