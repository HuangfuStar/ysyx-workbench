module top(
  input  [8:0] sw,
  output [15:0] ledr,
  output [7:0] seg0
);
  wire en;
  wire [7:0] din;
  reg valid;
  reg [2:0] code;
  reg [7:0] seg_data;

  assign en = sw[8];
  assign din = sw[7:0];

  always @(*) begin
    valid = 1'b0;
    code = 3'b000;

    if (en) begin
      casez (din)
        8'b1zzzzzzz: begin valid = 1'b1; code = 3'd7; end
        8'b01zzzzzz: begin valid = 1'b1; code = 3'd6; end
        8'b001zzzzz: begin valid = 1'b1; code = 3'd5; end
        8'b0001zzzz: begin valid = 1'b1; code = 3'd4; end
        8'b00001zzz: begin valid = 1'b1; code = 3'd3; end
        8'b000001zz: begin valid = 1'b1; code = 3'd2; end
        8'b0000001z: begin valid = 1'b1; code = 3'd1; end
        8'b00000001: begin valid = 1'b1; code = 3'd0; end
        default: begin valid = 1'b0; code = 3'd0; end
      endcase
    end
  end

  always @(*) begin
    case (code)
      3'd0: seg_data = 8'b00000011;
      3'd1: seg_data = 8'b10011111;
      3'd2: seg_data = 8'b00100101;
      3'd3: seg_data = 8'b00001101;
      3'd4: seg_data = 8'b10011001;
      3'd5: seg_data = 8'b01001001;
      3'd6: seg_data = 8'b01000001;
      3'd7: seg_data = 8'b00011111;
      default: seg_data = 8'b11111111;
    endcase
  end

  assign ledr[2:0] = code;
  assign ledr[3] = en;
  assign ledr[4] = valid;
  assign ledr[15:5] = 11'b0;
  assign seg0 = valid ? seg_data : 8'b11111111;
endmodule
