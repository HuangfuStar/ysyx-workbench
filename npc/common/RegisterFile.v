/* verilator lint_off DECLFILENAME */
module RegisterFile #(ADDR_WIDTH = 5, DATA_WIDTH = 32) (
  input clk,
  input rst,
  input [ADDR_WIDTH-1:0] raddr1,
  input [ADDR_WIDTH-1:0] raddr2,
  output [DATA_WIDTH-1:0] rdata1,
  output [DATA_WIDTH-1:0] rdata2,
  input wen,
  input [ADDR_WIDTH-1:0] waddr,
  input [DATA_WIDTH-1:0] wdata
);
  localparam DEPTH = 1 << ADDR_WIDTH;

  reg [DATA_WIDTH-1:0] rf [0:DEPTH-1];
  integer i;

  always @(posedge clk) begin
    if (rst) begin
      for (i = 0; i < DEPTH; i = i + 1) begin
        rf[i] <= {DATA_WIDTH{1'b0}};
      end
    end else begin
      if (wen && (waddr != {ADDR_WIDTH{1'b0}})) begin
        rf[waddr] <= wdata;
      end
      rf[0] <= {DATA_WIDTH{1'b0}};
    end
  end

  assign rdata1 = (raddr1 == {ADDR_WIDTH{1'b0}}) ? {DATA_WIDTH{1'b0}} : rf[raddr1];
  assign rdata2 = (raddr2 == {ADDR_WIDTH{1'b0}}) ? {DATA_WIDTH{1'b0}} : rf[raddr2];
endmodule
/* verilator lint_on DECLFILENAME */
