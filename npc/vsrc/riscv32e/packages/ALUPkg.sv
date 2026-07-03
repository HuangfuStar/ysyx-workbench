package ALUPkg;
    typedef enum logic [3:0] {
        ALU_ADD, ALU_SUB,
        ALU_AND, ALU_OR, ALU_XOR,
        ALU_SLL, ALU_SRL, ALU_SRA,
        ALU_EQ, ALU_NE, ALU_LT, ALU_LTU, ALU_GE, ALU_GEU
    } ALUctr_t;
endpackage
