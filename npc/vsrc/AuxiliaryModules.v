// `timescale 1ns / 1ps
// module ps2_keyboard_model(
//     output reg ps2_clk,
//     output reg ps2_data
//     );
// parameter [31:0] kbd_clk_period = 60;
// initial ps2_clk = 1'b1;

// task kbd_sendcode;
//     input [7:0] code; // key to be sent
//     integer i;

//     reg[10:0] send_buffer;
//     begin
//         send_buffer[0]   = 1'b0;  // start bit
//         send_buffer[8:1] = code;  // code
//         send_buffer[9]   = ~(^code); // odd parity bit
//         send_buffer[10]  = 1'b1;  // stop bit
//         i = 0;
//         while( i < 11) begin
//             // set kbd_data
//             ps2_data = send_buffer[i];
//             #(kbd_clk_period/2) ps2_clk = 1'b0;
//             #(kbd_clk_period/2) ps2_clk = 1'b1;
//             i = i + 1;
//         end
//     end
// endtask

// endmodule


// module ps2_keyboard(clk,clrn,ps2_clk,ps2_data,data,
//                     ready,nextdata_n,overflow);
//     input clk,clrn,ps2_clk,ps2_data;
//     input nextdata_n;
//     output [7:0] data;
//     output reg ready;
//     output reg overflow;     // fifo overflow
//     // internal signal, for test
//     reg [9:0] buffer;        // ps2_data bits
//     reg [7:0] fifo[7:0];     // data fifo
//     reg [2:0] w_ptr,r_ptr;   // fifo write and read pointers
//     reg [3:0] count;  // count ps2_data bits
//     // detect falling edge of ps2_clk
//     reg [2:0] ps2_clk_sync;

//     always @(posedge clk) begin
//         ps2_clk_sync <=  {ps2_clk_sync[1:0],ps2_clk};
//     end

//     wire sampling = ps2_clk_sync[2] & ~ps2_clk_sync[1];

//     always @(posedge clk) begin
//         if (clrn == 0) begin // reset
//             count <= 0; w_ptr <= 0; r_ptr <= 0; overflow <= 0; ready<= 0;
//         end
//         else begin
//             if ( ready ) begin // read to output next data
//                 if(nextdata_n == 1'b0) //read next data
//                 begin
//                     r_ptr <= r_ptr + 3'b1;
//                     if(w_ptr==(r_ptr+1'b1)) //empty
//                         ready <= 1'b0;
//                 end
//             end
//             if (sampling) begin
//               if (count == 4'd10) begin
//                 if ((buffer[0] == 0) &&  // start bit
//                     (ps2_data)       &&  // stop bit
//                     (^buffer[9:1])) begin      // odd  parity
//                     fifo[w_ptr] <= buffer[8:1];  // kbd scan code
//                     w_ptr <= w_ptr+3'b1;
//                     ready <= 1'b1;
//                     overflow <= overflow | (r_ptr == (w_ptr + 3'b1));
//                 end
//                 count <= 0;     // for next
//               end else begin
//                 buffer[count] <= ps2_data;  // store ps2_data
//                 count <= count + 3'b1;
//               end
//             end
//         end
//     end
//     assign data = fifo[r_ptr]; //always set output data

// endmodule

// module MuxKeyInternal #(NR_KEY = 2, KEY_LEN = 1, DATA_LEN = 1, HAS_DEFAULT = 0) (
//   output reg [DATA_LEN-1:0] out,
//   input [KEY_LEN-1:0] key,
//   input [DATA_LEN-1:0] default_out,
//   input [NR_KEY*(KEY_LEN + DATA_LEN)-1:0] lut
// );

//   localparam PAIR_LEN = KEY_LEN + DATA_LEN;
//   wire [PAIR_LEN-1:0] pair_list [NR_KEY-1:0];
//   wire [KEY_LEN-1:0] key_list [NR_KEY-1:0];
//   wire [DATA_LEN-1:0] data_list [NR_KEY-1:0];

//   generate
//     for (genvar n = 0; n < NR_KEY; n = n + 1) begin
//       assign pair_list[n] = lut[PAIR_LEN*(n+1)-1 : PAIR_LEN*n];
//       assign data_list[n] = pair_list[n][DATA_LEN-1:0];
//       assign key_list[n]  = pair_list[n][PAIR_LEN-1:DATA_LEN];
//     end
//   endgenerate

//   reg [DATA_LEN-1 : 0] lut_out;
//   reg hit;
//   integer i;
//   always @(*) begin
//     lut_out = 0;
//     hit = 0;
//     for (i = 0; i < NR_KEY; i = i + 1) begin
//       lut_out = lut_out | ({DATA_LEN{key == key_list[i]}} & data_list[i]);
//       hit = hit | (key == key_list[i]);
//     end
//     if (!HAS_DEFAULT) out = lut_out;
//     else out = (hit ? lut_out : default_out);
//   end

// endmodule

// module MuxKey #(NR_KEY = 2, KEY_LEN = 1, DATA_LEN = 1) (
//   output [DATA_LEN-1:0] out,
//   input [KEY_LEN-1:0] key,
//   input [NR_KEY*(KEY_LEN + DATA_LEN)-1:0] lut
// );
//   MuxKeyInternal #(NR_KEY, KEY_LEN, DATA_LEN, 0) i0 (out, key, {DATA_LEN{1'b0}}, lut);
// endmodule

// module MuxKeyWithDefault #(NR_KEY = 2, KEY_LEN = 1, DATA_LEN = 1) (
//   output [DATA_LEN-1:0] out,
//   input [KEY_LEN-1:0] key,
//   input [DATA_LEN-1:0] default_out,
//   input [NR_KEY*(KEY_LEN + DATA_LEN)-1:0] lut
// );
//   MuxKeyInternal #(NR_KEY, KEY_LEN, DATA_LEN, 1) i0 (out, key, default_out, lut);
// endmodule