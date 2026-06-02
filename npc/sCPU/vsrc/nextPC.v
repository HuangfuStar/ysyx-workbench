module nextPC(
    input [3:0] pc,
    input JMP,
    input JMPFlag,
    input [3:0] Addr,
    output [3:0] PCNext
);
    wire [3:0] tmpPC;
    wire key;

    assign tmpPC = pc + 4'h1;
    assign key = JMP & JMPFlag;

    MuxKey #(2, 1, 4) u_npc(PCNext, key, {
        1'b0, tmpPC,
        1'b1, Addr
    });
endmodule
