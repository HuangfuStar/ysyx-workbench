module sCPU(
    input clk,
    input rst,
    output [3:0] pc,
    output [7:0] inst
);
    wire [3:0] pc_next;
    wire [1:0] rs1;
    wire [1:0] rs2;
    wire [1:0] rd;
    wire [1:0] opcode;
    wire [3:0] addr;
    wire [3:0] imm;
    wire [7:0] rdata1;
    wire [7:0] rdata2;
    wire [7:0] alu_b;
    wire [7:0] alu_c;
    wire [7:0] reg_wdata;
    wire [1:0] raddr1;
    wire branch_taken;
    wire RegWE;
    wire RegSource;
    wire JMP;
    wire SubCtr;
    wire imm2Reg;
    /* verilator lint_off UNUSEDSIGNAL */
    wire alu_zf_unused;
    wire alu_of_unused;
    wire alu_cf_unused;
    /* verilator lint_on UNUSEDSIGNAL */

    Reg #(4, 4'h0) u_pc(
        .clk(clk),
        .rst(rst),
        .din(pc_next),
        .dout(pc),
        .wen(1'b1)
    );

    ROM u_rom(
        .addr(pc),
        .data(inst)
    );

    instDec u_dec(
        .inst(inst),
        .rs1(rs1),
        .rs2(rs2),
        .rd(rd),
        .opcode(opcode),
        .addr(addr),
        .imm(imm)
    );

    cu u_cu(
        .op(opcode),
        .RegWE(RegWE),
        .RegSource(RegSource),
        .JMP(JMP),
        .SubCtr(SubCtr),
        .imm2Reg(imm2Reg)
    );

    MuxKey #(2, 1, 2) u_raddr1(
        raddr1, JMP, {
            1'b0, rs1,
            1'b1, 2'b00
        }
    );

    regFile u_rf(
        .clk(clk),
        .rst(rst),
        .rdata1(rdata1),
        .rdata2(rdata2),
        .raddr1(raddr1),
        .raddr2(rs2),
        .WE(RegWE),
        .waddr(rd),
        .wdata(reg_wdata)
    );

    MuxKey #(2, 1, 8) u_alu_b(
        alu_b, imm2Reg, {
            1'b0, rdata2,
            1'b1, {4'b0000, imm}
        }
    );

    alu u_alu(
        .C(alu_c),
        .ZF(alu_zf_unused),
        .OF(alu_of_unused),
        .CF(alu_cf_unused),
        .A(rdata1),
        .B(alu_b),
        .SubCtr(SubCtr)
    );

    MuxKey #(2, 1, 8) u_wdata(
        reg_wdata, RegSource, {
            1'b0, alu_c,
            1'b1, {4'b0000, imm}
        }
    );

    assign branch_taken = |(rdata1 ^ rdata2);

    nextPC u_npc(
        .pc(pc),
        .JMP(JMP),
        .JMPFlag(branch_taken),
        .Addr(addr),
        .PCNext(pc_next)
    );
endmodule
