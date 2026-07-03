module DM(
  input  logic        clk_in,
  input  logic [31:0] addr_in,
  input  logic [31:0] wdata_in,
  input  logic        dm_re_in,
  input  logic        dm_we_in,
  input  logic        dm_wsize_in,
  input  logic        dm_rsize_in,
  output logic [31:0] rdata_out
);
  import "DPI-C" function int  pmem_read(input int raddr);
  import "DPI-C" function void pmem_write(input int waddr, input int wdata, input byte wmask);

  logic [31:0] w_word_data;
  logic [7:0]  w_byte_lane_data;
  logic [31:0] w_write_data;
  logic [4:0]  w_shift_bits;
  logic [3:0]  w_mask_data;

  assign w_word_data      = dm_re_in ? pmem_read(addr_in) : 32'b0;
  assign w_shift_bits     = {addr_in[1:0], 3'b000};
  assign w_byte_lane_data = ({8{addr_in[1:0] == 2'b00}} & w_word_data[7:0])   |
                            ({8{addr_in[1:0] == 2'b01}} & w_word_data[15:8])  |
                            ({8{addr_in[1:0] == 2'b10}} & w_word_data[23:16]) |
                            ({8{addr_in[1:0] == 2'b11}} & w_word_data[31:24]);
  assign rdata_out        = dm_rsize_in ? w_word_data : {24'b0, w_byte_lane_data};

  assign w_mask_data  = dm_wsize_in ? 4'b1111 : (4'b0001 << addr_in[1:0]);
  assign w_write_data = dm_wsize_in ? wdata_in : (wdata_in << w_shift_bits);

  always_ff @(posedge clk_in) begin
    if (dm_we_in) begin
      pmem_write(addr_in, w_write_data, {4'b0000, w_mask_data});
    end
  end
endmodule
