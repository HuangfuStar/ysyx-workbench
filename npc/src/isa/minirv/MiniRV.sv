module MiniRV(
  input logic clk_in,
  input logic rst_in
);
  import MiniRVPkg::*;
  import "DPI-C" function void npc_ebreak(input int pc, input int code);

  logic [31:0] w_pc_cur;
  logic [31:0] w_pc_next;
  logic [31:0] w_pc_plus4;
  logic [31:0] w_jmp_target;

  logic [31:0] w_inst_data;
  logic [6:0]  w_opcode_data;
  logic [2:0]  w_funct3_data;
  logic [6:0]  w_funct7_data;
  logic [4:0]  w_rs1_addr;
  logic [4:0]  w_rs2_addr;
  logic [4:0]  w_rd_addr;
  logic [31:0] w_iimm_data;
  logic [31:0] w_simm_data;
  logic [31:0] w_uimm_data;

  logic [31:0] w_gpr_rdata1;
  logic [31:0] w_gpr_rdata2;
  logic [31:0] w_gpr_wdata;

  logic [31:0] w_jimm_unused;

  logic [31:0] w_alu_src1;
  logic [31:0] w_alu_src2;
  logic [31:0] w_alu_result;
  
  logic [31:0] w_dm_rdata;
  logic [31:0] w_imm_data;
  logic [31:0] w_a0_data;
  logic        w_dm_re;
  logic        w_is_ebreak;
  CtrSignal_t  w_ctr_signal;

  IR u_IR (
    .clk_in(clk_in),
    .rst_in(rst_in),
    .next_pc_in(w_pc_next),
    .pc_out(w_pc_cur)
  );

  IM u_IM (
    .rst_in(rst_in),
    .pc_in(w_pc_cur),
    .inst_out(w_inst_data)
  );

  InstExtract u_InstExtract (
    .inst_in(w_inst_data),
    .opcode_out(w_opcode_data),
    .funct3_out(w_funct3_data),
    .funct7_out(w_funct7_data),
    .rs1_out(w_rs1_addr),
    .rs2_out(w_rs2_addr),
    .rd_out(w_rd_addr),
    .iimm_out(w_iimm_data),
    .simm_out(w_simm_data),
    .uimm_out(w_uimm_data),
    .jimm_out(w_jimm_unused)
  );

  CU u_CU (
    .funct3_in(w_funct3_data),
    .funct7_in(w_funct7_data),
    .opcode_in(w_opcode_data),
    .ctr_cu_signal_out(w_ctr_signal)
  );

  GPR u_GPR (
    .clk_in(clk_in),
    .rst_in(rst_in),
    .raddr1_in(w_rs1_addr),
    .raddr2_in(w_rs2_addr),
    .wdata_in(w_gpr_wdata),
    .waddr_in(w_rd_addr),
    .we_in(w_ctr_signal.ctr_GPR_we),
    .rdata1_out(w_gpr_rdata1),
    .rdata2_out(w_gpr_rdata2),
    .a0_out(w_a0_data)
  );

  ALU u_ALU (
    .a_in(w_alu_src1),
    .b_in(w_alu_src2),
    .result_out(w_alu_result)
  );

  DM u_DM (
    .clk_in(clk_in),
    .addr_in(w_alu_result),
    .wdata_in(w_gpr_rdata2),
    .dm_re_in(w_dm_re),
    .dm_we_in(w_ctr_signal.ctr_DM_we),
    .dm_wsize_in(w_ctr_signal.ctr_DM_wsize),
    .dm_rsize_in(w_ctr_signal.ctr_DM_rsize),
    .rdata_out(w_dm_rdata)
  );

  NextPC u_NextPC (
    .pc_in(w_pc_cur),
    .jmp_in(w_ctr_signal.ctr_JMP),
    .next_pc_in(w_jmp_target),
    .next_pc_out(w_pc_next)
  );

  assign w_imm_data = (w_opcode_data == OPCODE_LUI) ? w_uimm_data : w_iimm_data;
  assign w_alu_src1 = (w_opcode_data == OPCODE_LUI) ? 32'b0 : w_gpr_rdata1;
  assign w_alu_src2 = ({32{w_ctr_signal.ctr_ALU_src2 == ALU_SRC2_GPR}}  & w_gpr_rdata2) |
                      ({32{w_ctr_signal.ctr_ALU_src2 == ALU_SRC2_IIMM}} & w_iimm_data) |
                      ({32{w_ctr_signal.ctr_ALU_src2 == ALU_SRC2_SIMM}} & w_simm_data);
  assign w_pc_plus4 = w_pc_cur + 32'd4;
  assign w_dm_re = (w_ctr_signal.ctr_GPR_wdata == GPR_WDATA_DM);
  assign w_gpr_wdata = ({32{w_ctr_signal.ctr_GPR_wdata == GPR_WDATA_IMM}} & w_imm_data)   |
                       ({32{w_ctr_signal.ctr_GPR_wdata == GPR_WDATA_ALU}} & w_alu_result) |
                       ({32{w_ctr_signal.ctr_GPR_wdata == GPR_WDATA_DM}}  & w_dm_rdata)   |
                       ({32{w_ctr_signal.ctr_GPR_wdata == GPR_WDATA_PC4}} & w_pc_plus4);
  assign w_jmp_target = {w_alu_result[31:1], 1'b0} | (w_jimm_unused & 32'b0);
  assign w_is_ebreak = (w_opcode_data == OPCODE_SYSTEM) && (w_inst_data == EBREAK_INST);

`ifdef CONFIG_DEBUG
  export "DPI-C" function npc_get_pc;
  export "DPI-C" function npc_get_gpr;
  export "DPI-C" function npc_get_gpr_num;
  export "DPI-C" function npc_get_inst;
  export "DPI-C" function npc_get_next_pc;
  export "DPI-C" function npc_get_mem_valid;
  export "DPI-C" function npc_get_mem_is_write;
  export "DPI-C" function npc_get_mem_addr;
  export "DPI-C" function npc_get_mem_wdata;
  export "DPI-C" function npc_get_mem_rdata;
  export "DPI-C" function npc_get_mem_len;

  function int npc_get_pc();
    npc_get_pc = w_pc_cur;
  endfunction

  function int npc_get_gpr(input int idx);
    if (idx < 0 || idx >= 32) begin
      npc_get_gpr = 0;
    end else begin
      npc_get_gpr = u_GPR.u_RegisterFile.rf[idx[4:0]];
    end
  endfunction

  function int npc_get_gpr_num();
    npc_get_gpr_num = 32;
  endfunction

  function int npc_get_inst();
    npc_get_inst = w_inst_data;
  endfunction

  function int npc_get_next_pc();
    npc_get_next_pc = w_pc_next;
  endfunction

  function int npc_get_mem_valid();
    npc_get_mem_valid = w_dm_re || w_ctr_signal.ctr_DM_we;
  endfunction

  function int npc_get_mem_is_write();
    npc_get_mem_is_write = w_ctr_signal.ctr_DM_we;
  endfunction

  function int npc_get_mem_addr();
    npc_get_mem_addr = w_alu_result;
  endfunction

  function int npc_get_mem_wdata();
    npc_get_mem_wdata = w_gpr_rdata2;
  endfunction

  function int npc_get_mem_rdata();
    npc_get_mem_rdata = w_dm_rdata;
  endfunction

  function int npc_get_mem_len();
    npc_get_mem_len = (w_dm_re || w_ctr_signal.ctr_DM_wsize || w_ctr_signal.ctr_DM_rsize) ?
                      ((w_ctr_signal.ctr_DM_wsize || w_ctr_signal.ctr_DM_rsize) ? 4 : 1) : 0;
  endfunction
`endif

  always_ff @(posedge clk_in) begin
    if (!rst_in && w_is_ebreak) begin
      npc_ebreak(w_pc_cur, w_a0_data);
    end
  end
endmodule
