module top(
    input btn,
    output [7:0] ledr,
    output [7:0] seg1,
    output [7:0] seg0
);

    logic [7:0] data = 8'b0000_0001;
    logic [7:0] seg0_data;
    logic [7:0] seg1_data;

    always_ff @(posedge btn) begin
        data <= {data[0] ^ data[2] ^ data[3] ^ data[4], data[7:1]};
    end

    seg7_hex u_seg0(
        .x(data[3:0]),
        .seg(seg0_data)
    );
    seg7_hex u_seg1(
        .x(data[7:4]),
        .seg(seg1_data)
    );

    assign ledr = data;
    assign seg0 = seg0_data;
    assign seg1 = seg1_data;

endmodule
