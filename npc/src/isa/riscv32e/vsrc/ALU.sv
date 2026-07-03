module ALU (
    input  logic [31:0] A,
    input  logic [31:0] B,
    input  ALUPkg::ALUctr_t ALUctr,
    output logic [31:0] C
);
    import ALUPkg::*;

    always_comb begin
        C = 32'b0;

        case (ALUctr)
            ALU_ADD: C = A + B;
            ALU_SUB: C = A - B;
            ALU_AND: C = A & B;
            ALU_OR : C = A | B;
            ALU_XOR: C = A ^ B;

            ALU_SLL: C = A << B[4:0];
            ALU_SRL: C = A >> B[4:0];
            ALU_SRA: C = $signed(A) >>> B[4:0];

            ALU_EQ : C = {31'b0, (A == B)};
            ALU_NE : C = {31'b0, (A != B)};

            ALU_LT : C = {31'b0, ($signed(A) < $signed(B))};
            ALU_LTU: C = {31'b0, (A < B)};

            ALU_GE : C = {31'b0, ($signed(A) >= $signed(B))};
            ALU_GEU: C = {31'b0, (A >= B)};

            default: C = 32'b0;
        endcase
    end
endmodule
