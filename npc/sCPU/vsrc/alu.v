module alu(
    output [7:0] C,
    output ZF,
    output OF,
    output CF,
    input [7:0] A,
    input [7:0] B,
    input SubCtr
);
    wire [7:0] B_;
    wire [8:0] carry;

    assign carry[0] = SubCtr;

    MuxKey #(2, 1, 8) u_bin(B_, SubCtr, {
        1'b0, B,
        1'b1, ~B
    });

    genvar i;
    generate
        for (i = 0; i < 8; i = i + 1) begin : gen_c
            FullAdder u_fa(
                .s(C[i]),
                .cout(carry[i + 1]),
                .a(A[i]),
                .b(B_[i]),
                .cin(carry[i])
            );
        end
    endgenerate

    assign ZF = ~|C;
    assign CF = carry[8] ^ SubCtr;
    assign OF = carry[7] ^ carry[8];
    
endmodule

/* verilator lint_off DECLFILENAME */
module FullAdder(
    output s,
    output cout,
    input a,
    input b,
    input cin
);
    assign s = a ^ b ^ cin;
    assign cout = (a & b) | (a & cin) | (b & cin);
endmodule
/* verilator lint_on DECLFILENAME */
