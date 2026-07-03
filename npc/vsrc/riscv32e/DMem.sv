module DMem (
    input  logic        clk,
    input  logic [31:0] addr,
    input  logic        re,
    input  logic        we,
    input  logic [2:0]  funct3,
    input  logic [31:0] wdata,
    output logic [31:0] rdata
);
    import "DPI-C" function int pmem_read(input int raddr);
    import "DPI-C" function void pmem_write(input int waddr, input int wdata, input byte wmask);

    logic [31:0] word_data;
    logic [31:0] shifted_wdata;
    /* verilator lint_off UNUSEDSIGNAL */
    logic [31:0] shifted_byte_word;
    logic [31:0] shifted_half_word;
    /* verilator lint_on UNUSEDSIGNAL */
    logic [3:0]  wmask;
    logic [4:0]  byte_shift_bits;
    logic [4:0]  half_shift_bits;
    logic [7:0]  byte_data;
    logic [15:0] half_data;

    assign word_data = re ? pmem_read(addr) : 32'b0;

    assign byte_shift_bits = {addr[1:0], 3'b000};
    assign half_shift_bits = {addr[1], 4'b0000};

    assign shifted_byte_word = word_data >> byte_shift_bits;
    assign shifted_half_word = word_data >> half_shift_bits;

    assign byte_data = shifted_byte_word[7:0];
    assign half_data = shifted_half_word[15:0];
    // TODO: load write align implementation: exception or hardware handled
    // read data generation 
    always_comb begin
        rdata = 32'b0;
        unique case (funct3)
            3'b000: rdata = {{24{byte_data[7]}}, byte_data};  // lb
            3'b001: rdata = {{16{half_data[15]}}, half_data}; // lh
            3'b010: rdata = word_data;                        // lw
            3'b100: rdata = {24'b0, byte_data};               // lbu
            3'b101: rdata = {16'b0, half_data};               // lhu
            default: rdata = 32'b0;
        endcase
    end

    always_comb begin
        wmask = 4'b0000;

        unique case (funct3)
            3'b000: wmask = 4'b0001 << addr[1:0];
            3'b001: wmask = addr[1] ? 4'b1100 : 4'b0011;
            3'b010: wmask = 4'b1111;
            default: wmask = 4'b0000;
        endcase
    end

    always_comb begin
        shifted_wdata = 32'b0;

        unique case (funct3)
            3'b000: shifted_wdata = wdata << byte_shift_bits;
            3'b001: shifted_wdata = wdata << half_shift_bits;
            3'b010: shifted_wdata = wdata;
            default: shifted_wdata = 32'b0;
        endcase
    end

    always_ff @(posedge clk) begin
        if (we) begin
            pmem_write(addr, shifted_wdata, {4'b0000, wmask});
        end
    end
endmodule
