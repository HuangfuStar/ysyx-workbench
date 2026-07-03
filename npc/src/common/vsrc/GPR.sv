module GPR #(
  parameter ADDR_WIDTH = 5,
  parameter RF_ADDR_WIDTH = 5,
  parameter DATA_WIDTH = 32,
  parameter A0_ADDR = 10
) (
  input  logic                  clk_in,
  input  logic                  rst_in,
  input  logic [ADDR_WIDTH-1:0] raddr1_in,
  input  logic [ADDR_WIDTH-1:0] raddr2_in,
  input  logic [DATA_WIDTH-1:0] wdata_in,
  input  logic [ADDR_WIDTH-1:0] waddr_in,
  input  logic                  we_in,
  output logic [DATA_WIDTH-1:0] rdata1_out,
  output logic [DATA_WIDTH-1:0] rdata2_out,
  output logic [DATA_WIDTH-1:0] a0_out
);
  logic [RF_ADDR_WIDTH-1:0] w_raddr1;
  logic [RF_ADDR_WIDTH-1:0] w_raddr2;
  logic [RF_ADDR_WIDTH-1:0] w_raddr3;
  logic [RF_ADDR_WIDTH-1:0] w_waddr;
  logic [DATA_WIDTH-1:0]     w_rdata1;
  logic [DATA_WIDTH-1:0]     w_rdata2;
  logic [DATA_WIDTH-1:0]     w_rdata3;
  logic                     w_raddr1_valid;
  logic                     w_raddr2_valid;
  logic                     w_raddr3_valid;
  logic                     w_waddr_valid;

  function automatic logic addr_valid(input logic [ADDR_WIDTH-1:0] addr);
    integer i;
    begin
      addr_valid = 1'b1;
      for (i = RF_ADDR_WIDTH; i < ADDR_WIDTH; i = i + 1) begin
        addr_valid &= ~addr[i];
      end
    end
  endfunction

  assign w_raddr1_valid = addr_valid(raddr1_in);
  assign w_raddr2_valid = addr_valid(raddr2_in);
  assign w_raddr3_valid = addr_valid(A0_ADDR[ADDR_WIDTH-1:0]);
  assign w_waddr_valid  = addr_valid(waddr_in);
  assign w_raddr1 = raddr1_in[RF_ADDR_WIDTH-1:0];
  assign w_raddr2 = raddr2_in[RF_ADDR_WIDTH-1:0];
  assign w_raddr3 = A0_ADDR[RF_ADDR_WIDTH-1:0];
  assign w_waddr  = waddr_in[RF_ADDR_WIDTH-1:0];

  RegisterFile #(
    .ADDR_WIDTH(RF_ADDR_WIDTH),
    .DATA_WIDTH(DATA_WIDTH)
  ) u_RegisterFile (
    .clk(clk_in),
    .rst(rst_in),
    .raddr1(w_raddr1),
    .raddr2(w_raddr2),
    .raddr3(w_raddr3),
    .rdata1(w_rdata1),
    .rdata2(w_rdata2),
    .rdata3(w_rdata3),
    .wen(we_in && w_waddr_valid),
    .waddr(w_waddr),
    .wdata(wdata_in)
  );

  assign rdata1_out = w_raddr1_valid ? w_rdata1 : {DATA_WIDTH{1'b0}};
  assign rdata2_out = w_raddr2_valid ? w_rdata2 : {DATA_WIDTH{1'b0}};
  assign a0_out     = w_raddr3_valid ? w_rdata3 : {DATA_WIDTH{1'b0}};
endmodule
