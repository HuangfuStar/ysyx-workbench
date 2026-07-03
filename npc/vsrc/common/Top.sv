module Top(
  input logic clk_in,
  input logic rst_in
);
`ifdef CONFIG_ISA_RISCV32E
  riscv32e u_core (
    .clk_in(clk_in),
    .rst_in(rst_in)
  );
`else
  MiniRV u_core (
    .clk_in(clk_in),
    .rst_in(rst_in)
  );
`endif
endmodule
