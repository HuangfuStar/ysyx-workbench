package MiniRVPkg;
  typedef struct packed {
    logic       ctr_GPR_we;
    logic [1:0] ctr_GPR_wdata;
    logic [1:0] ctr_ALU_src2;
    logic       ctr_DM_we;
    logic       ctr_DM_wsize;
    logic       ctr_DM_rsize;
    logic       ctr_JMP;
  } CtrSignal_t;

  localparam logic [6:0] OPCODE_OP     = 7'b0110011;
  localparam logic [6:0] OPCODE_OPIMM  = 7'b0010011;
  localparam logic [6:0] OPCODE_LUI    = 7'b0110111;
  localparam logic [6:0] OPCODE_LOAD   = 7'b0000011;
  localparam logic [6:0] OPCODE_STORE  = 7'b0100011;
  localparam logic [6:0] OPCODE_JALR   = 7'b1100111;
  localparam logic [6:0] OPCODE_SYSTEM = 7'b1110011;

  localparam logic [1:0] GPR_WDATA_IMM = 2'b00;
  localparam logic [1:0] GPR_WDATA_ALU = 2'b01;
  localparam logic [1:0] GPR_WDATA_DM  = 2'b10;
  localparam logic [1:0] GPR_WDATA_PC4 = 2'b11;

  localparam logic [1:0] ALU_SRC2_GPR  = 2'b00;
  localparam logic [1:0] ALU_SRC2_IIMM = 2'b01;
  localparam logic [1:0] ALU_SRC2_SIMM = 2'b10;

  localparam logic [31:0] RESET_PC     = 32'h0000_0000;
  localparam logic [31:0] EBREAK_INST  = 32'h0010_0073;
endpackage
