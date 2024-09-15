module ex03ALU(
    input [2:0] sel,//选择运算符
    input [3:0] a,b,//输入运算数
    output reg [4:0] out,//输出结果
    output OF,ZF,CF,SF,//
    output [7:0] outseg0,outseg1//输出字符
);

alu alu01//alu模块
(
    .sel(sel),
    .a(a),
    .b(b),
    .out(out),
    .OF(OF),
    .ZF(ZF),
    .CF(CF),
    .SF(SF)
);

// 打印seg0
bcd7seg seg0(
    .b(out[2:0]),
    .h(outseg0)
);

//打印seg1
bcd7seg seg1(
    .b({1'b0,out[4:3]}),
    .h(outseg1)
);



endmodule


module alu(
    input [2:0] sel,
    input [3:0] a,b,
    output reg [4:0] out,
    output OF,ZF,CF,SF
);

wire  [3:0] outn,outor,outxor,outand,subb;//反、或、异或、数或被减数
wire  [4:0] outps;//加减
reg         ifsub;//是否减
wire        ifps;//是否进位
wire        AlargeB;//A>b

assign out=5'b0;
assign subb=(ifsub) ? (~b+1):b;
assign outn=~a;
assign outor=a|b;
assign outxor=a^b;
assign outand=a&b;
assign outps=a+subb;
assign ifps=outps[4];

assign SF=outps[3];
assign CF=ifps;//进位
assign OF=ifps^outps[3];//溢出
assign ZF=((a+~b+1)==4'b0) ? 1:0;//是否为0

assign AlargeB=outps[3]^OF;

always @(*) begin
    ifsub=0;
    out=5'b0;
    case(sel)
    3'b000:begin//加法
        ifsub=0;
        out=outps;
    end
    3'b001:begin//减法
        ifsub=1;
        out=outps;
    end
    3'b010:out[3:0]=outn;//反
    3'b011:out[3:0]=outand;//与
    3'b100:out[3:0]=outor;//或
    3'b101:out[3:0]=outxor;//异或
    3'b110:begin//A与B比较
        ifsub=1;
        if(AlargeB) out=0;
        else out=1;
    end
    3'b111:begin//是否为相等
        ifsub=1;
        if(ZF) out=1;
        else out=0;
    end
endcase
end

endmodule

