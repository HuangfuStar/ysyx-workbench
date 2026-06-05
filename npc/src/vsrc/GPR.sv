module GPR(
  input  logic        clk_in,
  input  logic        rst_in,
  input  logic [4:0]  raddr1_in,
  input  logic [4:0]  raddr2_in,
  input  logic [31:0] wdata_in,
  input  logic [4:0]  waddr_in,
  input  logic        we_in,
  output logic [31:0] rdata1_out,
  output logic [31:0] rdata2_out,
  output logic [31:0] a0_out
);
  logic [31:0] w_gpr_data [0:31];
  integer i;

  always_ff @(posedge clk_in) begin
    if (rst_in) begin
      for (i = 0; i < 32; i = i + 1) begin
        w_gpr_data[i] <= 32'b0;
      end
    end else begin
      if (we_in && (waddr_in != 5'b0)) begin
        w_gpr_data[waddr_in] <= wdata_in;
      end
      w_gpr_data[0] <= 32'b0;
    end
  end

  assign rdata1_out = (raddr1_in == 5'b0) ? 32'b0 : w_gpr_data[raddr1_in];
  assign rdata2_out = (raddr2_in == 5'b0) ? 32'b0 : w_gpr_data[raddr2_in];
  assign a0_out = w_gpr_data[10];
endmodule
