module InstExtract(
  input  logic [31:0] inst_in,
  output logic [6:0]  opcode_out,
  output logic [2:0]  funct3_out,
  output logic [6:0]  funct7_out,
  output logic [4:0]  rs1_out,
  output logic [4:0]  rs2_out,
  output logic [4:0]  rd_out,
  output logic [31:0] iimm_out,
  output logic [31:0] simm_out,
  output logic [31:0] uimm_out,
  output logic [31:0] jimm_out
);
  assign opcode_out = inst_in[6:0];
  assign rd_out     = inst_in[11:7];
  assign funct3_out = inst_in[14:12];
  assign rs1_out    = inst_in[19:15];
  assign rs2_out    = inst_in[24:20];
  assign funct7_out = inst_in[31:25];

  assign iimm_out = {{20{inst_in[31]}}, inst_in[31:20]};
  assign simm_out = {{20{inst_in[31]}}, inst_in[31:25], inst_in[11:7]};
  assign uimm_out = {inst_in[31:12], 12'b0};
  assign jimm_out = {{11{inst_in[31]}}, inst_in[31], inst_in[19:12], inst_in[20], inst_in[30:21], 1'b0};
endmodule
