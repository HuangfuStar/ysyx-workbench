module top(
    input clk,
    input rst,
    input ps2_clk,
    input ps2_data,
    output [7:0] seg0,
    output [7:0] seg1,
    output [7:0] seg2,
    output [7:0] seg3,
    output [7:0] seg4,
    output [7:0] seg5
);

    logic [7:0] ps2_byte;
    logic ps2_ready;
    logic ps2_nextdata_n;
    logic ps2_overflow;

    logic break_pending;
    logic key_active;
    logic [7:0] current_scan;
    logic [7:0] current_ascii;
    logic [7:0] press_count;

    logic [7:0] scan_or_blank;
    logic [7:0] ascii_or_blank;

    logic [7:0] seg0_data;
    logic [7:0] seg1_data;
    logic [7:0] seg2_data;
    logic [7:0] seg3_data;
    logic [7:0] seg4_data;
    logic [7:0] seg5_data;
    logic [7:0] ascii_lookup;
    wire is_supported = (ascii_lookup != 8'h00);

    ps2_keyboard u_keyboard(
        .clk(clk),
        .clrn(~rst),
        .ps2_clk(ps2_clk),
        .ps2_data(ps2_data),
        .data(ps2_byte),
        .ready(ps2_ready),
        .nextdata_n(ps2_nextdata_n),
        .overflow(ps2_overflow)
    );

    ascii_rom u_ascii_rom(
        .addr(ps2_byte),
        .data(ascii_lookup)
    );

    always_ff @(posedge clk) begin
        if (rst) begin
            ps2_nextdata_n <= 1'b1;
            break_pending <= 1'b0;
            key_active <= 1'b0;
            current_scan <= 8'h00;
            current_ascii <= 8'h00;
            press_count <= 8'h00;
        end else begin
            ps2_nextdata_n <= 1'b1;

            if (ps2_ready && ps2_nextdata_n) begin
                ps2_nextdata_n <= 1'b0;

                if (ps2_byte == 8'hF0) begin
                    break_pending <= 1'b1;
                end else if (break_pending) begin
                    break_pending <= 1'b0;
                    key_active <= 1'b0;
                    current_scan <= 8'h00;
                    current_ascii <= 8'h00;
                end else if (is_supported && !key_active) begin
                    key_active <= 1'b1;
                    current_scan <= ps2_byte;
                    current_ascii <= ascii_lookup;
                    press_count <= press_count + 8'h01;
                end
            end
        end
    end

    always_comb begin
        if (key_active) begin
            scan_or_blank = current_scan;
            ascii_or_blank = current_ascii;
        end else begin
            scan_or_blank = 8'hFF;
            ascii_or_blank = 8'hFF;
        end
    end

    seg7_hex u_seg0(
        .x(scan_or_blank[3:0]),
        .seg(seg0_data)
    );
    seg7_hex u_seg1(
        .x(scan_or_blank[7:4]),
        .seg(seg1_data)
    );
    seg7_hex u_seg2(
        .x(ascii_or_blank[3:0]),
        .seg(seg2_data)
    );
    seg7_hex u_seg3(
        .x(ascii_or_blank[7:4]),
        .seg(seg3_data)
    );
    seg7_hex u_seg4(
        .x(press_count[3:0]),
        .seg(seg4_data)
    );
    seg7_hex u_seg5(
        .x(press_count[7:4]),
        .seg(seg5_data)
    );

    assign seg0 = key_active ? seg0_data : 8'hFF;
    assign seg1 = key_active ? seg1_data : 8'hFF;
    assign seg2 = key_active ? seg2_data : 8'hFF;
    assign seg3 = key_active ? seg3_data : 8'hFF;
    assign seg4 = seg4_data;
    assign seg5 = seg5_data;

endmodule
