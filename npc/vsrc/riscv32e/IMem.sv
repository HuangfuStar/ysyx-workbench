module IMem (
    input  logic        rst,
    input  logic [31:0] addr,
    output logic [31:0] inst
);
    import "DPI-C" function int pmem_read(input int raddr);

    assign inst = rst ? 32'h0000_0013 : pmem_read(addr);
endmodule
