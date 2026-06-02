module regFile(
    input clk,
    input rst,
    output [7:0] rdata1,
    output [7:0] rdata2,
    input [1:0] raddr1,
    input [1:0] raddr2,
    input WE,
    input [1:0] waddr,
    input [7:0] wdata
);

wire [7:0] rf [3:0];
wire [3:0] wen;

genvar i;
generate
    for (i = 0; i < 4; i = i + 1) begin : gen_rf
        localparam [1:0] IDX = i;
        assign wen[i] = WE & (waddr == IDX);
        Reg #(8, 8'h00) u_reg (
            .clk(clk),
            .rst(rst),
            .din(wdata),
            .dout(rf[i]),
            .wen(wen[i])
        );
    end
endgenerate

MuxKey #(4, 2, 8) u_rdata1 (
    rdata1, raddr1, {
        2'h0, rf[0],
        2'h1, rf[1],
        2'h2, rf[2],
        2'h3, rf[3]
    }
);

MuxKey #(4, 2, 8) u_rdata2 (
    rdata2, raddr2, {
        2'h0, rf[0],
        2'h1, rf[1],
        2'h2, rf[2],
        2'h3, rf[3]
    }
);

endmodule
