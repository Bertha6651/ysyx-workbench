module ex06RegisterOrShifter(
    input clk,
    output reg [7:0] radom ,
    output [7:0] seg01,seg02
);
radomNumGenerator step01(
    .clk(clk),
    .radom(radom),
    .seg01(seg01),
    .seg02(seg02)
);
endmodule

// module shiftRegister(
//     input shamt[2:0],
//     input din[7:0],
//     input clk,

//     output dout[7:0]
// ); 

// endmodule

module radomNumGenerator //生成随机数
(
    input clk,
    output reg [7:0] radom,
    output [7:0] seg01,seg02
) ;
    wire temp;
    assign temp=radom[4]^radom[3]^radom[2]^radom[0];//这里的思路是直接参考的讲义上的
    always @(posedge clk) begin
        if(radom==8'b0) begin
            radom<=8'b1;
        end
        else begin
            radom<={temp,radom[7:1]};
        end
    end

    bcd15seg seg1(
        .b(radom[3:0]),
        .h(seg01[7:0])
    );
    bcd15seg seg2(
        .b(radom[7:4]),
        .h(seg02[7:0])
    );
endmodule



module bcd15seg//打印16进制数
(  
  input  [3:0] b,  
  output reg [7:0] h  
);  

always @(*) begin  
    case(b)  
        4'h0: h = ~8'b11111101;  
        4'h1: h = ~8'b01100000;  
        4'h2: h = ~8'b11011010;  
        4'h3: h = ~8'b11110010;  
        4'h4: h = ~8'b01100110;  
        4'h5: h = ~8'b10110110;  
        4'h6: h = ~8'b10111110;  
        4'h7: h = ~8'b11100000;   
        4'h8: h = ~8'b11111110;  
        4'h9: h = ~8'b11110110;  
        4'hA: h = ~8'b11101110;  
        4'hB: h = ~8'b11111110;  
        4'hC: h = ~8'b10011100;  
        4'hD: h = ~8'b11111100;  
        4'hE: h = ~8'b10011110;  
        4'hF: h = ~8'b10001110;  
        default: h = 8'b00000010;  
    endcase  
end  

endmodule
