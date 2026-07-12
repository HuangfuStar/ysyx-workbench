module riscv32e(
    input logic clk_in,
    input logic rst_in
);
    import ALUPkg::*;
    import NextPCPkg::*;
    import RV32EPkg::*;
    import "DPI-C" function void npc_ebreak(input int pc, input int code);

    logic [31:0] pc;
    logic [31:0] pc_next;
    logic [31:0] pc_step;
    logic [31:0] inst;

    logic [6:0]  opcode;
    logic [2:0]  funct3;
    logic [6:0]  funct7;
    logic [4:0]  rs1;
    logic [4:0]  rs2;
    logic [4:0]  rd;

    logic [31:0] iimm;
    logic [31:0] simm;
    logic [31:0] bimm;
    logic [31:0] uimm;
    logic [31:0] jimm;
    logic [11:0] csr_addr;
    logic [31:0] csr_uimm;

    logic [31:0] rs1_data;
    logic [31:0] rs2_data;
    logic [31:0] a0_data;
    logic [31:0] gpr_wdata;
    logic        gpr_we;

    logic [31:0] alu_src1;
    logic [31:0] alu_src2;
    logic [31:0] alu_result;
    logic [31:0] mem_rdata;
    logic [31:0] csr_rdata;
    logic [31:0] csr_wdata;
    logic [31:0] csr_mtvec;
    logic [31:0] csr_mepc;
    logic        branch_taken;
    logic        csr_we;
    logic        trap_we;
    logic [31:0] trap_cause;

    ALUctr_t     alu_ctr;
    NextPCctr_t  nextpc_ctr;
    GPRWbSel_t   wb_sel;
    ALUSrc2Sel_t alu_src2_sel;

    logic is_lui;
    logic is_auipc;
    logic is_jal;
    logic is_jalr;
    logic is_branch;
    logic is_load;
    logic is_store;
    logic is_opimm;
    logic is_op;
    logic is_system;
    logic is_csr;
    logic is_csrrw;
    logic is_csrrs;
    logic is_csrrc;
    logic is_csrrwi;
    logic is_csrrsi;
    logic is_csrrci;
    logic is_csr_imm;
    logic is_ecall;
    logic is_ebreak;
    logic is_mret;

    // split the instruction
    assign opcode   = inst[6:0];
    assign rd       = inst[11:7];
    assign funct3   = inst[14:12];
    assign rs1      = inst[19:15];
    assign rs2      = inst[24:20];
    assign funct7   = inst[31:25];
    assign csr_addr = inst[31:20];

    // construct the immediate
    assign iimm = {{20{inst[31]}}, inst[31:20]};
    assign simm = {{20{inst[31]}}, inst[31:25], inst[11:7]};
    assign bimm = {{19{inst[31]}}, inst[31], inst[7], inst[30:25], inst[11:8], 1'b0};
    assign uimm = {inst[31:12], 12'b0};
    assign jimm = {{11{inst[31]}}, inst[31], inst[19:12], inst[20], inst[30:21], 1'b0};
    assign csr_uimm = {27'b0, rs1};

    // decode the opocode
    assign is_lui    = (opcode == OPCODE_LUI);
    assign is_auipc  = (opcode == OPCODE_AUIPC);
    assign is_jal    = (opcode == OPCODE_JAL);
    assign is_jalr   = (opcode == OPCODE_JALR)   && (funct3 == FUNCT3_JALR);
    assign is_branch = (opcode == OPCODE_BRANCH);
    assign is_load   = (opcode == OPCODE_LOAD);
    assign is_store  = (opcode == OPCODE_STORE);
    assign is_opimm  = (opcode == OPCODE_OPIMM);
    assign is_op     = (opcode == OPCODE_OP);
    assign is_system = (opcode == OPCODE_SYSTEM);

    // detailed docoding 
    assign is_csrrw  = is_system && (funct3 == FUNCT3_CSRRW);
    assign is_csrrs  = is_system && (funct3 == FUNCT3_CSRRS);
    assign is_csrrc  = is_system && (funct3 == FUNCT3_CSRRC);
    assign is_csrrwi = is_system && (funct3 == FUNCT3_CSRRWI);
    assign is_csrrsi = is_system && (funct3 == FUNCT3_CSRRSI);
    assign is_csrrci = is_system && (funct3 == FUNCT3_CSRRCI);
    assign is_csr_imm = is_csrrwi || is_csrrsi || is_csrrci;
    assign is_csr    = is_csrrw || is_csrrs || is_csrrc || is_csrrwi || is_csrrsi || is_csrrci;

    assign is_ecall  = is_system && (inst == ECALL_INST);
    assign is_ebreak = is_system && (inst == EBREAK_INST);
    assign is_mret   = is_system && (inst == MRET_INST);

    // GPR write back selection signal
    always_comb begin
        wb_sel = GPR_WB_ALU;
        if (is_load) begin
            wb_sel = GPR_WB_MEM;
        end else if (is_jal || is_jalr) begin
            wb_sel = GPR_WB_PC4;
        end else if (is_lui) begin
            wb_sel = GPR_WB_IMM;
        end else if (is_csr) begin
            wb_sel = GPR_WB_CSR;
        end
    end

    // ALU src2 selection control signal
    always_comb begin
        alu_src2_sel = ALU_SRC_RS2;
        if (is_store) begin
            alu_src2_sel = ALU_SRC_SIMM;
        end else if (is_lui || is_auipc) begin
            alu_src2_sel = ALU_SRC_UIMM;
        end else if (is_opimm || is_load || is_jalr) begin
            alu_src2_sel = ALU_SRC_IIMM;
        end
    end

    // ALU controller signal generation
    always_comb begin
        alu_ctr = ALU_ADD;

        if (is_branch) begin
            unique case (funct3)
                FUNCT3_BEQ : alu_ctr = ALU_EQ;
                FUNCT3_BNE : alu_ctr = ALU_NE;
                FUNCT3_BLT : alu_ctr = ALU_LT;
                FUNCT3_BGE : alu_ctr = ALU_GE;
                FUNCT3_BLTU: alu_ctr = ALU_LTU;
                FUNCT3_BGEU: alu_ctr = ALU_GEU;
                default: alu_ctr = ALU_ADD;
            endcase
        end else if (is_op || is_opimm) begin
            unique case (funct3)
                FUNCT3_ADD_SUB: alu_ctr = (is_op && (funct7 == FUNCT7_SUB_SRA)) ? ALU_SUB : ALU_ADD;
                FUNCT3_SLL    : alu_ctr = ALU_SLL;
                FUNCT3_SLT    : alu_ctr = ALU_LT;
                FUNCT3_SLTU   : alu_ctr = ALU_LTU;
                FUNCT3_XOR    : alu_ctr = ALU_XOR;
                FUNCT3_SRL_SRA: alu_ctr = (funct7 == FUNCT7_SUB_SRA) ? ALU_SRA : ALU_SRL;
                FUNCT3_OR     : alu_ctr = ALU_OR;
                FUNCT3_AND    : alu_ctr = ALU_AND;
                default: alu_ctr = ALU_ADD;
            endcase
        end
    end
    
    // ALU data source 
    assign alu_src1 = is_auipc ? pc : rs1_data;
    assign alu_src2 =
        (alu_src2_sel == ALU_SRC_RS2)  ? rs2_data :
        (alu_src2_sel == ALU_SRC_IIMM) ? iimm :
        (alu_src2_sel == ALU_SRC_SIMM) ? simm :
                                         uimm;

    assign branch_taken = is_branch && alu_result[0];
    assign csr_wdata = is_csr_imm ? csr_uimm : rs1_data;

    // CSR write enable control signal
    assign csr_we =
        is_csrrw || is_csrrwi ||
        (is_csrrs  && (rs1 != 5'd0)) ||
        (is_csrrc  && (rs1 != 5'd0)) ||
        (is_csrrsi && (rs1 != 5'd0)) ||
        (is_csrrci && (rs1 != 5'd0));
    assign trap_we = is_ecall;
    assign trap_cause = MCAUSE_ECALL_M;

    always_comb begin
        nextpc_ctr = NEXTPC_A4;
        if (is_ecall) begin
            nextpc_ctr = NEXTPC_TRAP;
        end else if (is_mret) begin
            nextpc_ctr = NEXTPC_MRET;
        end else if (is_jal) begin
            nextpc_ctr = NEXTPC_JAL;
        end else if (is_jalr) begin
            nextpc_ctr = NEXTPC_JALR;
        end else if (is_branch) begin
            nextpc_ctr = NEXTPC_BRANCH;
        end
    end

    assign gpr_we = !rst_in && (rd < 5'd16) && (rd != 5'd0) &&
                    (is_lui || is_auipc || is_jal || is_jalr || is_load || is_opimm || is_op || is_csr);
    assign gpr_wdata =
        (wb_sel == GPR_WB_MEM) ? mem_rdata :
        (wb_sel == GPR_WB_PC4) ? pc_step :
        (wb_sel == GPR_WB_IMM) ? uimm :
        (wb_sel == GPR_WB_CSR) ? csr_rdata :
                                 alu_result;

    GPR #(
        .ADDR_WIDTH(5),
        .RF_ADDR_WIDTH(4),
        .DATA_WIDTH(32),
        .A0_ADDR(10)
    ) u_gpr (
        .clk_in(clk_in),
        .rst_in(rst_in),
        .raddr1_in(rs1),
        .raddr2_in(rs2),
        .wdata_in(gpr_wdata),
        .waddr_in(rd),
        .we_in(gpr_we),
        .rdata1_out(rs1_data),
        .rdata2_out(rs2_data),
        .a0_out(a0_data)
    );

    IMem u_imem (
        .rst(rst_in),
        .addr(pc),
        .inst(inst)
    );

    ALU u_alu (
        .A(alu_src1),
        .B(alu_src2),
        .ALUctr(alu_ctr),
        .C(alu_result)
    );

    DMem u_dmem (
        .clk(clk_in),
        .addr(alu_result),
        .re(is_load),
        .we(!rst_in && is_store),
        .funct3(funct3),
        .wdata(rs2_data),
        .rdata(mem_rdata)
    );

    CSR u_csr (
        .clk_in(clk_in),
        .rst_in(rst_in),
        .addr_in(csr_addr),
        .we_in(csr_we),
        .cmd_in(funct3),
        .wdata_in(csr_wdata),
        .trap_we_in(trap_we),
        .trap_pc_in(pc),
        .trap_cause_in(trap_cause),
        .rdata_out(csr_rdata),
        .mtvec_out(csr_mtvec),
        .mepc_out(csr_mepc)
    );

    NextPC u_nextpc (
        .pc(pc),
        .rs1(rs1_data),
        .iimm(iimm),
        .Bcond(branch_taken),
        .bimm(bimm),
        .jimm(jimm),
        .mtvec(csr_mtvec),
        .mepc(csr_mepc),
        .nextPCctr(nextpc_ctr),
        .pcs_next(pc_step),
        .pcd_next(pc_next)
    );

`ifdef CONFIG_DEBUG
    export "DPI-C" function npc_get_pc;
    export "DPI-C" function npc_get_gpr;
    export "DPI-C" function npc_get_gpr_num;
    export "DPI-C" function npc_get_inst;
    export "DPI-C" function npc_get_next_pc;
    export "DPI-C" function npc_get_mem_valid;
    export "DPI-C" function npc_get_mem_is_write;
    export "DPI-C" function npc_get_mem_addr;
    export "DPI-C" function npc_get_mem_wdata;
    export "DPI-C" function npc_get_mem_rdata;
    export "DPI-C" function npc_get_mem_len;

    function int npc_get_pc();
        npc_get_pc = pc;
    endfunction

    function int npc_get_gpr(input int idx);
        if (idx < 0 || idx >= 16) begin
            npc_get_gpr = 0;
        end else begin
            npc_get_gpr = u_gpr.u_RegisterFile.rf[idx[3:0]];
        end
    endfunction

    function int npc_get_gpr_num();
        npc_get_gpr_num = 16;
    endfunction

    function int npc_get_inst();
        npc_get_inst = inst;
    endfunction

    function int npc_get_next_pc();
        npc_get_next_pc = pc_next;
    endfunction

    function int npc_get_mem_valid();
        npc_get_mem_valid = {31'b0, (is_load || is_store)};
    endfunction

    function int npc_get_mem_is_write();
        npc_get_mem_is_write = {31'b0, is_store};
    endfunction

    function int npc_get_mem_addr();
        npc_get_mem_addr = alu_result;
    endfunction

    function int npc_get_mem_wdata();
        npc_get_mem_wdata = rs2_data;
    endfunction

    function int npc_get_mem_rdata();
        npc_get_mem_rdata = mem_rdata;
    endfunction

    function int npc_get_mem_len();
        if (is_load || is_store) begin
            unique case (funct3)
                3'b000: npc_get_mem_len = 1;
                3'b001: npc_get_mem_len = 2;
                default: npc_get_mem_len = 4;
            endcase
        end else begin
            npc_get_mem_len = 0;
        end
    endfunction
`endif

    always_ff @(posedge clk_in) begin
        if (rst_in) begin
            pc <= RESET_PC;
        end else begin
            pc <= pc_next;
            if (is_ebreak) begin
                npc_ebreak(pc, a0_data);
            end
        end
    end
endmodule
