module CSR (
    input  logic        clk_in,
    input  logic        rst_in,
    input  logic [11:0] addr_in,
    input  logic        we_in,
    input  logic [2:0]  cmd_in,
    input  logic [31:0] wdata_in,
    input  logic        trap_we_in,
    input  logic [31:0] trap_pc_in,
    input  logic [31:0] trap_cause_in,
    output logic [31:0] rdata_out,
    output logic [31:0] mtvec_out,
    output logic [31:0] mepc_out
);
    import RV32EPkg::*;

    logic [63:0] mcycle_q;
    logic [63:0] mcycle_d;
    logic [31:0] mstatus_q;
    logic [31:0] mtvec_q;
    logic [31:0] mepc_q;
    logic [31:0] mcause_q;

    function automatic logic [31:0] csr_write_data(
        input logic [31:0] old_value,
        input logic [31:0] write_value,
        input logic [2:0]  cmd
    );
        begin
            unique case (cmd)
                FUNCT3_CSRRW,
                FUNCT3_CSRRWI: csr_write_data = write_value;
                FUNCT3_CSRRS,
                FUNCT3_CSRRSI: csr_write_data = old_value | write_value;
                FUNCT3_CSRRC,
                FUNCT3_CSRRCI: csr_write_data = old_value & ~write_value;
                default      : csr_write_data = old_value;
            endcase
        end
    endfunction

    always_comb begin
        unique case (addr_in)
            CSR_MVENDORID: rdata_out = MVENDORID_VALUE;
            CSR_MARCHID  : rdata_out = MARCHID_VALUE;
            CSR_MSTATUS  : rdata_out = mstatus_q;
            CSR_MTVEC    : rdata_out = mtvec_q;
            CSR_MEPC     : rdata_out = mepc_q;
            CSR_MCAUSE   : rdata_out = mcause_q;
            CSR_MCYCLE   : rdata_out = mcycle_q[31:0];
            CSR_MCYCLEH  : rdata_out = mcycle_q[63:32];
            default      : rdata_out = 32'b0;
        endcase
    end

    assign mtvec_out = mtvec_q;
    assign mepc_out  = mepc_q;

    always_comb begin
        mcycle_d = mcycle_q + 64'd1;

        if (we_in) begin
            unique case (addr_in)
                CSR_MCYCLE : mcycle_d[31:0]  = csr_write_data(mcycle_q[31:0], wdata_in, cmd_in);
                CSR_MCYCLEH: mcycle_d[63:32] = csr_write_data(mcycle_q[63:32], wdata_in, cmd_in);
                default: begin
                end
            endcase
        end
    end

    always_ff @(posedge clk_in) begin
        if (rst_in) begin
            mcycle_q  <= 64'b0;
            mstatus_q <= MSTATUS_RESET_VALUE;
            mtvec_q   <= 32'b0;
            mepc_q    <= 32'b0;
            mcause_q  <= 32'b0;
        end else begin
            mcycle_q <= mcycle_d;

            if (we_in) begin
                unique case (addr_in)
                    CSR_MSTATUS: mstatus_q <= csr_write_data(mstatus_q, wdata_in, cmd_in);
                    CSR_MTVEC  : mtvec_q   <= csr_write_data(mtvec_q, wdata_in, cmd_in);
                    CSR_MEPC   : mepc_q    <= csr_write_data(mepc_q, wdata_in, cmd_in);
                    CSR_MCAUSE : mcause_q  <= csr_write_data(mcause_q, wdata_in, cmd_in);
                    default: begin
                    end
                endcase
            end

            if (trap_we_in) begin
                mepc_q   <= trap_pc_in;
                mcause_q <= trap_cause_in;
            end
        end
    end
endmodule
