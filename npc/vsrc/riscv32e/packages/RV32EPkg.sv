package RV32EPkg;
    typedef enum logic [2:0] {
        GPR_WB_ALU,
        GPR_WB_MEM,
        GPR_WB_PC4,
        GPR_WB_IMM,
        GPR_WB_CSR
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

    localparam logic [2:0] FUNCT3_CSRRW  = 3'b001;
    localparam logic [2:0] FUNCT3_CSRRS  = 3'b010;
    localparam logic [2:0] FUNCT3_CSRRC  = 3'b011;
    localparam logic [2:0] FUNCT3_CSRRWI = 3'b101;
    localparam logic [2:0] FUNCT3_CSRRSI = 3'b110;
    localparam logic [2:0] FUNCT3_CSRRCI = 3'b111;

    localparam logic [11:0] CSR_MVENDORID = 12'hF11;
    localparam logic [11:0] CSR_MARCHID   = 12'hF12;
    localparam logic [11:0] CSR_MSTATUS   = 12'h300;
    localparam logic [11:0] CSR_MTVEC     = 12'h305;
    localparam logic [11:0] CSR_MEPC      = 12'h341;
    localparam logic [11:0] CSR_MCAUSE    = 12'h342;
    localparam logic [11:0] CSR_MCYCLE  = 12'hB00;
    localparam logic [11:0] CSR_MCYCLEH = 12'hB80;

    localparam logic [31:0] MVENDORID_VALUE = 32'h7973_7978;
    localparam logic [31:0] MARCHID_VALUE   = 32'h0150_BE98;
    localparam logic [31:0] MSTATUS_RESET_VALUE = 32'h0000_1800;

    localparam logic [31:0] RESET_PC    = 32'h8000_0000;
    localparam logic [31:0] ECALL_INST  = 32'h0000_0073;
    localparam logic [31:0] EBREAK_INST = 32'h0010_0073;
    localparam logic [31:0] MRET_INST   = 32'h3020_0073;
    localparam logic [31:0] MCAUSE_ECALL_M = 32'd11;
endpackage
