module IR(
  input  logic        clk_in,
  input  logic        rst_in,
  input  logic [31:0] next_pc_in,
  output logic [31:0] pc_out
);
  import MiniRVPkg::*;

  always_ff @(posedge clk_in) begin
    if (rst_in) begin
      pc_out <= RESET_PC;
    end else begin
      pc_out <= next_pc_in;
    end
  end
endmodule
