module CU(
  input  logic [2:0] funct3_in,
  input  logic [6:0] funct7_in,
  input  logic [6:0] opcode_in,
  output MiniRVPkg::CtrSignal_t ctr_cu_signal_out
);
  import MiniRVPkg::*;

  logic w_is_add;
  logic w_is_addi;
  logic w_is_lui;
  logic w_is_lw;
  logic w_is_lbu;
  logic w_is_sw;
  logic w_is_sb;
  logic w_is_jalr;
  logic       w_ctr_GPR_we;
  logic [1:0] w_ctr_GPR_wdata;
  logic [1:0] w_ctr_ALU_src2;
  logic       w_ctr_DM_we;
  logic       w_ctr_DM_wsize;
  logic       w_ctr_DM_rsize;
  logic       w_ctr_JMP;

  assign w_is_add  = (opcode_in == OPCODE_OP)    && (funct3_in == 3'b000) && (funct7_in == 7'b0000000);
  assign w_is_addi = (opcode_in == OPCODE_OPIMM) && (funct3_in == 3'b000);
  assign w_is_lui  = (opcode_in == OPCODE_LUI);
  assign w_is_lw   = (opcode_in == OPCODE_LOAD)  && (funct3_in == 3'b010);
  assign w_is_lbu  = (opcode_in == OPCODE_LOAD)  && (funct3_in == 3'b100);
  assign w_is_sw   = (opcode_in == OPCODE_STORE) && (funct3_in == 3'b010);
  assign w_is_sb   = (opcode_in == OPCODE_STORE) && (funct3_in == 3'b000);
  assign w_is_jalr = (opcode_in == OPCODE_JALR)  && (funct3_in == 3'b000);

  assign w_ctr_GPR_we    = w_is_add || w_is_addi || w_is_lui || w_is_lw || w_is_lbu || w_is_jalr;
  assign w_ctr_GPR_wdata = ({2{w_is_lw || w_is_lbu}}      & GPR_WDATA_DM)  |
                           ({2{w_is_jalr}}                & GPR_WDATA_PC4) |
                           ({2{w_is_add || w_is_addi}}    & GPR_WDATA_ALU) |
                           ({2{w_is_lui}}                 & GPR_WDATA_IMM);
  assign w_ctr_ALU_src2  = ({2{w_is_addi || w_is_lw || w_is_lbu || w_is_jalr}} & ALU_SRC2_IIMM) |
                           ({2{w_is_sw || w_is_sb}}                             & ALU_SRC2_SIMM);
  assign w_ctr_DM_we     = w_is_sw || w_is_sb;
  assign w_ctr_DM_wsize  = w_is_sw;
  assign w_ctr_DM_rsize  = w_is_lw;
  assign w_ctr_JMP       = w_is_jalr;
  assign ctr_cu_signal_out = '{
    ctr_GPR_we:    w_ctr_GPR_we,
    ctr_GPR_wdata: w_ctr_GPR_wdata,
    ctr_ALU_src2:  w_ctr_ALU_src2,
    ctr_DM_we:     w_ctr_DM_we,
    ctr_DM_wsize:  w_ctr_DM_wsize,
    ctr_DM_rsize:  w_ctr_DM_rsize,
    ctr_JMP:       w_ctr_JMP
  };
endmodule
