module NextPC (
    input  logic [31:0] pc,
    input  logic [31:0] rs1,
    input  logic [31:0] iimm,
    input  logic        Bcond,
    input  logic [31:0] bimm,
    input  logic [31:0] jimm,
    input  NextPCPkg::NextPCctr_t nextPCctr,

    output logic [31:0] pcs_next,
    output logic [31:0] pcd_next
);
    import NextPCPkg::*;

    assign pcs_next = pc + 32'd4;

    assign pcd_next =
        (nextPCctr == NEXTPC_A4)   ? pcs_next :
        (nextPCctr == NEXTPC_JAL)  ? (pc + jimm) :
        (nextPCctr == NEXTPC_JALR) ? ((rs1 + iimm) & ~32'b1) :
        (nextPCctr == NEXTPC_BRANCH && Bcond)
                                    ? (pc + bimm)
                                    : pcs_next;

endmodule
