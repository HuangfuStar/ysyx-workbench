module Top(
  input logic clk_in,
  input logic rst_in
);
  MiniRV u_MiniRV (
    .clk_in(clk_in),
    .rst_in(rst_in)
  );
endmodule
