module ex02DeOrEncoder(
    input [7:0] in,
    // input resetn ,
    output reg indicator,
    output reg [2:0] out,
    output [7:0] seg
);


// reg [2:0] i;
// assign  resetn = 1;

always @(*) begin
    // i=7;
    // out=3'b0;
    // indicator=0;
    // // if(!resetn)  out=3'b0;
    // // else 
    // // begin
    // for(i = 7;in[i]==0 ;i--)
    // begin
    //     if(in[i]==0 && i==0) begin
    //         indicator=0;
    //         out=0;
    //     end
    //     else begin 
    //         indicator=1;
    //         out=i;
    //     end
    // end
    // // end 
    indicator =1;
    out=0;
    casez (in)
        8'b1zzzzzzz: out=7;
        8'b01zzzzzz: out=6;
        8'b001zzzzz: out=5;
        8'b0001zzzz: out=4;
        8'b00001zzz: out=3;
        8'b000001zz: out=2;
        8'b0000001z: out=1;
        8'b00000001: out=0;
        default: indicator=0;
    endcase
end
bcd7seg seg38(
    .b(out),
    .h(seg));


endmodule


module bcd7seg(
  input  [2:0] b,
  output reg [7:0] h
);
// detailed implementation ...
always @(*) begin
    case(b)
        3'd0:h=~8'b11111101;
        3'd1:h=~8'b01100000;
        3'd2:h=~8'b11011010;
        3'd3:h=~8'b11110010;
        3'd4:h=~8'b01100110;
        3'd5:h=~8'b10110110;
        3'd6:h=~8'b10111110;
        3'd7:h=~8'b11100000;
        default:h=~8'b11111101;
    endcase

end
endmodule


