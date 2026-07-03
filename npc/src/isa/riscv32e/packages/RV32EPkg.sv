package RV32EPkg;
    typedef enum logic [1:0] {
        GPR_WB_ALU,
        GPR_WB_MEM,
        GPR_WB_PC4,
        GPR_WB_IMM
    } GPRWbSel_t;

    typedef enum logic [1:0] {
        ALU_SRC_RS2,
        ALU_SRC_IIMM,
        ALU_SRC_SIMM,
        ALU_SRC_UIMM
    } ALUSrc2Sel_t;
    /*
    Table 70. RISC-V base opcode map, inst[1:0]=11

                    inst[4:2]
    inst[6:5]   |   000   |   001    |   010      |   011      |   100    |   101   |     110          |   111 (>32b)
    ---------------------------------------------------------------------------------------------------------------
    00          | *LOAD   | LOAD-FP  | custom-0   | MISC-MEM   | OP-IMM   |*AUIPC   |*OP-IMM-32       |   48b
    01          | *STORE  | STORE-FP | custom-1   | AMO        | OP       |*LUI     |*OP-32           |   64b
    10          |  MADD   | MSUB     | NMSUB      | NMADD      | OP-FP    | OP-V    | custom-2/rv128  |   48b
    11          |*BRANCH  |*JALR     | reserved   |*JAL        |*SYSTEM   | OP-VE   | custom-3/rv128  |   ≥80b
    */
    localparam logic [6:0] OPCODE_LUI    = 7'b0110111;
    localparam logic [6:0] OPCODE_AUIPC  = 7'b0010111;
    localparam logic [6:0] OPCODE_JAL    = 7'b1101111;
    localparam logic [6:0] OPCODE_JALR   = 7'b1100111;
    localparam logic [6:0] OPCODE_BRANCH = 7'b1100011;
    localparam logic [6:0] OPCODE_LOAD   = 7'b0000011;
    localparam logic [6:0] OPCODE_STORE  = 7'b0100011;
    localparam logic [6:0] OPCODE_OPIMM  = 7'b0010011;
    localparam logic [6:0] OPCODE_OP     = 7'b0110011;
    localparam logic [6:0] OPCODE_SYSTEM = 7'b1110011;

    localparam logic [31:0] RESET_PC    = 32'h8000_0000;
    localparam logic [31:0] EBREAK_INST = 32'h0010_0073;
endpackage
