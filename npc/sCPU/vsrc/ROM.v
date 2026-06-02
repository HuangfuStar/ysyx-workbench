
/*
 * 16-byte ROM
 * 4-bit address
 * 8-bit data
 */
module ROM(
  input  [3:0] addr,
  output [7:0] data
);
  reg [7:0] rom [0:15];

  initial begin
    rom[ 0] = 8'h81; // li   r0, 1
    rom[ 1] = 8'h90; // li   r1, 0
    rom[ 2] = 8'hab; // li   r2, 11
    rom[ 3] = 8'hb1; // li   r3, 1
    rom[ 4] = 8'h14; // add  r1, r1, r0
    rom[ 5] = 8'h03; // add  r0, r0, r3
    rom[ 6] = 8'hd2; // bner0 4, r2
    rom[ 7] = 8'hdd; // bner0 7, r1
    rom[ 8] = 8'h00;
    rom[ 9] = 8'h00;
    rom[10] = 8'h00;
    rom[11] = 8'h00;
    rom[12] = 8'h00;
    rom[13] = 8'h00;
    rom[14] = 8'h00;
    rom[15] = 8'h00;
  end

  assign data = rom[addr];
endmodule
