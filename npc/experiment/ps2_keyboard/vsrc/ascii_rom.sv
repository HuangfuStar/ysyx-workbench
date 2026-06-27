module ascii_rom(
    input  [7:0] addr,
    output [7:0] data
);

    reg [7:0] rom [0:255];
    integer i;

    initial begin
        for (i = 0; i < 256; i = i + 1) begin
            rom[i] = 8'h00;
        end

        rom[8'h45] = 8'h30;
        rom[8'h16] = 8'h31;
        rom[8'h1E] = 8'h32;
        rom[8'h26] = 8'h33;
        rom[8'h25] = 8'h34;
        rom[8'h2E] = 8'h35;
        rom[8'h36] = 8'h36;
        rom[8'h3D] = 8'h37;
        rom[8'h3E] = 8'h38;
        rom[8'h46] = 8'h39;

        rom[8'h1C] = 8'h41;
        rom[8'h32] = 8'h42;
        rom[8'h21] = 8'h43;
        rom[8'h23] = 8'h44;
        rom[8'h24] = 8'h45;
        rom[8'h2B] = 8'h46;
        rom[8'h34] = 8'h47;
        rom[8'h33] = 8'h48;
        rom[8'h43] = 8'h49;
        rom[8'h3B] = 8'h4A;
        rom[8'h42] = 8'h4B;
        rom[8'h4B] = 8'h4C;
        rom[8'h3A] = 8'h4D;
        rom[8'h31] = 8'h4E;
        rom[8'h44] = 8'h4F;
        rom[8'h4D] = 8'h50;
        rom[8'h15] = 8'h51;
        rom[8'h2D] = 8'h52;
        rom[8'h1B] = 8'h53;
        rom[8'h2C] = 8'h54;
        rom[8'h3C] = 8'h55;
        rom[8'h2A] = 8'h56;
        rom[8'h1D] = 8'h57;
        rom[8'h22] = 8'h58;
        rom[8'h35] = 8'h59;
        rom[8'h1A] = 8'h5A;
    end

    assign data = rom[addr];

endmodule
