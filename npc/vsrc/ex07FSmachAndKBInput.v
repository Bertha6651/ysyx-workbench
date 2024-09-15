module ex07FSmachAndKBInput (
    input clk,
    input rst,
    input ps2_clk,
    input ps2_data,
    output reg ifBreak,//数码管控制端口,判断是否释放按键
    output reg shift,alt,ctrl,
    // output reg combKey,
    output[7:0] seg01,seg02,seg11,seg12,segInitial,seg31,seg32,seg33//在七段LED数码管显示
  );
  reg  [7:0 ]ascii;
  reg ready,nextdata_n,overFlow;
  reg [7:0] ps2_out;
  reg [23:0] ps2_in ;

    // 键盘控制器
  ps2_keyboard ps2keyboard(.clk(clk),
                           .clrn(~rst), //低电平复位
                           .ps2_clk(ps2_clk),
                           .ps2_data(ps2_data),
                           .data(ps2_out),
                           .ready(ready),
                           .nextdata_n(nextdata_n),
                           .overflow(overFlow)
                          );




    // 三段式状态机部分

  parameter s01Read = 4'b0001;
  parameter s02Finish01 = 4'b0010; //拉低nextdata_n,通知读取完毕
  parameter s03Finish02 = 4'b0100;//拉低nextdata_n,通知读取完毕
  parameter s04Iitn = 4'b1000;


  reg [3:0] state_current,state_next;
  //状态机第一段
  always @(posedge clk or posedge rst)
  begin

    if (rst) state_current<=s04Iitn;

    else  state_current<=state_next;
  end
  //状态机第二段
  always @(*)
  begin
    case (state_current)
      s04Iitn:state_next=(ready==1'b1)?s01Read:s04Iitn;

      s01Read:state_next=s02Finish01;

      s02Finish01:state_next=s03Finish02;

      s03Finish02:state_next=s04Iitn;

      default:state_next=s04Iitn;
    endcase
  end
  //状态机第三段，信号输出逻辑
  always @(posedge clk)
  begin
    case (state_current)
      
      s01Read:ps2_in[23:0] <={ps2_in[15:0],ps2_out[7:0]};//保存读取的最后三个值

      s04Iitn:nextdata_n<=1;//升高
 
      s02Finish01:nextdata_n<=0;//降低

      s03Finish02:nextdata_n<=0;//降低

      default:
        nextdata_n<=1;//默认升高
     
    endcase
  end






//   always @(*)
//   begin
//     if (ps2_in[15:8]==8'hf0&&ps2_in[23:16]==ps2_in[7:0]^(shift||ctrl))// 断码形式：(A,0XF0,A)，ifBreak来显示
//     begin
//       ifBreak = 0;
//     end 
//     else 
//     begin
//       ifBreak = 1;
//     end
    

//   end


// always @(*)
// begin
//   if ((ps2_in[7:0]==8'h12))//是否shift+组合键
//     begin
//       shift = 1;
//     end
//     else shift=shift;
    

//     if ((ps2_in[7:0]==8'h14))//是否ctrl+组合键
//     begin
//       ctrl = 1;
//     end
//     else ctrl=ctrl;
// end

// always @(*)
// begin
//   if (~ifBreak)begin
//     shift=0;
//     ctrl=0;
//   end else begin
//     shift=shift;
//     ctrl=ctrl;
//   end
// end

// 时序逻辑块：使用时钟边沿检测shift和ctrl信号，并清除ifBreak信号
always @(posedge clk or negedge rst) begin
    if (rst) begin
        shift <= 0;
        ctrl <= 0;
        ifBreak <= 0;
        // combKey<=1;
    end else begin
        // 检测shift键按下
        if (ps2_in[7:0] == 8'h12 && ps2_in[15:8] != 8'hf0) begin//shift
            shift <= 1;
        end else if (ps2_in[7:0] == 8'h14 && ps2_in[15:8] != 8'hf0) begin//ctrl
          ctrl <= 1;
        // end else if (!combKey) begin
        // end else if ((ps2_in[15:8] == 8'hf0 && ((ps2_in[7:0] == 8'h14)||(ps2_in[7:0]==8'h12)))&& (shift || ctrl)) begin 
        
        //   shift <= 0;
        end else if (ps2_in[7:0] == 8'h11 && ps2_in[15:8] != 8'hf0) begin//ctrl
          alt <= 1;
        end else begin
          shift<=shift;
          ctrl<=ctrl;
          alt<=alt;
          // ctrl<=ctrl;
        end
        
        // if (!combKey) begin
        //   shift <= 0;
        //   combKey<=0;
        //   // ctrl<=0;
        // end else begin
        //   combKey<=combKey;
        //   shift<=shift; 
        //   // ctrl<=ctrl;
        // end

        // 检测ctrl键按下
        // if (ps2_in[7:0] == 8'h14) begin
        //     combKey<=1;
        //     ctrl <= 1;
        // end else if (~combKey) begin
        //   ctrl <= 0;
        //   combKey<=0;
        // end else begin
        //   ctrl<=ctrl;
        //   combKey<=combKey;
        // end
        

        // 断码检测：如果为断码形式则ifBreak置0，否则置1
        if (ps2_in[15:8] == 8'hf0 && ps2_in[23:16] == ps2_in[7:0])//普通键松开
        begin   
          if(ps2_in[7:0]==8'h12) shift<=0;
          if(ps2_in[7:0]==8'h14) ctrl<=0;
          if(ps2_in[7:0]==8'h11) alt<=0;
          ifBreak <= 0;
        end else if ((ps2_in[15:8] == 8'hf0 && ps2_in[7:0] == 8'h12) && shift)//组合键shift、ctrl松开
        begin     
          ifBreak <=0;
          shift <= 0;
        end else if ((ps2_in[15:8] == 8'hf0 && (ps2_in[7:0]==8'h14))&&  ctrl)//组合键shift、ctrl松开
        begin
          ifBreak<=0;
          ctrl<=0;
        end else if ((ps2_in[15:8] == 8'hf0 && (ps2_in[7:0]==8'h11))&& alt)//组合键shift、ctrl松开
        begin
          ifBreak<=0;
          alt<=0;
        end else begin
            ifBreak <= 1;
        end
    end
end

  // 记录按下键盘的次数
  reg [3:0] count [2:0];

  initial begin//初始count
    count[0]=4'b0;
    count[1]=4'b0;
    count[2]=4'b0;
  end

  always @(negedge ifBreak)
  begin
    if (count[1]==4'd9)begin   //进百位
      count[2]<=count[2]+4'd1;
      count[1]<=4'd0;
    end if (count[0]==4'd9) begin  //进十位
        count[1] <=  count[1] +4'd1;
        count[0] <= 4'd0;
    end  else  begin
        count[0] <= count[0] +4'd1;  //进个位
    end
  end

  


  // 七段LED数码管显示 
  
    //扫描码显示
  bcd15segIfrst segScan01 (.in(ps2_in[3:0]), .out(seg01),.en(ifBreak ));
  bcd15segIfrst segScan02 (.in(ps2_in[7:4]), .out(seg02),.en(ifBreak));
  
    // ASCII 码显示 

      // 扫描码转换为ASCII
  
  ScantoASCII  toascii(.addr(ps2_in[7:0]),.val(ascii),.shift(shift));

      // ASCII显示
  bcd15segIfrst segAscii01 (.in(ascii[3:0]), .out(seg11),.en(ifBreak ));
  bcd15segIfrst segAscii02 (.in(ascii[7:4]), .out(seg12),.en(ifBreak));
      
      //空白数码管初始化
  bcd15segIfrst segIinital (.in(count[0]), .out(segInitial),.en(1'd0));

    // 计数显示 
  bcd15segIfrst segNum01 (.in(count[0]), .out(seg31),.en(1'd1 ));
  bcd15segIfrst segNum02 (.in(count[1]), .out(seg32),.en(1'd1));
  bcd15segIfrst segNum03 (.in(count[2]), .out(seg33),.en(1'd1));



endmodule


module bcd15segIfrst(  //这里主要是加了一个复位功能
  input  [3:0] in,  
  input  en,
  output reg [7:0] out  
);  

always @(*) begin  
    if(~en) out = ~8'b11111101;
    else begin
    case(in)  
        4'h0: out = ~8'b11111101;  
        4'h1: out = ~8'b01100000;  
        4'h2: out = ~8'b11011010;  
        4'h3: out = ~8'b11110010;  
        4'h4: out = ~8'b01100110;  
        4'h5: out = ~8'b10110110;  
        4'h6: out = ~8'b10111110;  
        4'h7: out = ~8'b11100000;   
        4'h8: out = ~8'b11111110;  
        4'h9: out = ~8'b11110110;  
        4'hA: out = ~8'b11101110;  
        4'hB: out = ~8'b11111110;  
        4'hC: out = ~8'b10011100;  
        4'hD: out = ~8'b11111100;  
        4'hE: out = ~8'b10011110;  
        4'hF: out = ~8'b10001110;  
        default: out = 8'b00000010;  
    endcase  
    end
end  

endmodule

// module Scan (
//     input [7:0] addr,
//     output reg [7:0] val
// );

// always @(*) begin
//     case(addr)
//     #待补充
//     #注意:你需要自己搜索键盘对应的ASCII值
//     #键盘扫描码:键盘对应的ASCII值;
//     default:val=8'b0;

//     endcase
// end

// end
// endmodule


module ScantoASCII (//Scan码对应ASCII
    input [7:0] addr,
    input shift,
    output reg [7:0] val
);

always @(*) begin
    if (shift) begin 
        case (addr)
            8'h15: val = 8'h51; // Q
            8'h1D: val = 8'h57; // W
            8'h24: val = 8'h45; // E
            8'h2D: val = 8'h52; // R
            8'h2C: val = 8'h54; // T
            8'h35: val = 8'h59; // Y
            8'h3C: val = 8'h55; // U
            8'h43: val = 8'h49; // I
            8'h44: val = 8'h4F; // O
            8'h4D: val = 8'h50; // P
            8'h1C: val = 8'h41; // A
            8'h1B: val = 8'h53; // S
            8'h23: val = 8'h44; // D
            8'h2B: val = 8'h46; // F
            8'h34: val = 8'h47; // G
            8'h33: val = 8'h48; // H
            8'h3B: val = 8'h4A; // J
            8'h42: val = 8'h4B; // K
            8'h4B: val = 8'h4C; // L
            8'h1A: val = 8'h5A; // Z
            8'h22: val = 8'h58; // X
            8'h21: val = 8'h43; // C
            8'h2A: val = 8'h56; // V
            8'h32: val = 8'h42; // B
            8'h31: val = 8'h4E; // N
            8'h3A: val = 8'h4D; // M
            8'h29: val = 8'h20; // Space
            8'h0E: val = 8'h7E; // ~ (Shift + `)
            8'h16: val = 8'h21; // ! (Shift + 1)
            8'h1E: val = 8'h40; // @ (Shift + 2)
            8'h26: val = 8'h23; // # (Shift + 3)
            8'h25: val = 8'h24; // $ (Shift + 4)
            8'h2E: val = 8'h25; // % (Shift + 5)
            8'h36: val = 8'h5E; // ^ (Shift + 6)
            8'h3D: val = 8'h26; // & (Shift + 7)
            8'h3E: val = 8'h2A; // * (Shift + 8)
            8'h46: val = 8'h28; // ( (Shift + 9)
            8'h45: val = 8'h29; // ) (Shift + 0)
            8'h4E: val = 8'h5F; // _ (Shift + -)
            8'h55: val = 8'h2B; // + (Shift + =)
            8'h66: val = 8'h08; // Backspace
            8'h54: val = 8'h7B; // { (Shift + [)
            8'h5B: val = 8'h7D; // } (Shift + ])
            8'h4C: val = 8'h3A; // : (Shift + ;)
            8'h52: val = 8'h22; // " (Shift + ')
            8'h41: val = 8'h3C; // < (Shift + ,)
            8'h49: val = 8'h3E; // > (Shift + .)
            8'h4A: val = 8'h3F; // ? (Shift + /)
            8'h5D: val = 8'h7C; // | (Shift + \)
            default: val = 8'b0; 
        endcase
    end else begin
        case (addr)
            8'h15: val = 8'h71; // q
            8'h1D: val = 8'h77; // w
            8'h24: val = 8'h65; // e
            8'h2D: val = 8'h72; // r
            8'h2C: val = 8'h74; // t
            8'h35: val = 8'h79; // y
            8'h3C: val = 8'h75; // u
            8'h43: val = 8'h69; // i
            8'h44: val = 8'h6F; // o
            8'h4D: val = 8'h70; // p
            8'h1C: val = 8'h61; // a
            8'h1B: val = 8'h73; // s
            8'h23: val = 8'h64; // d
            8'h2B: val = 8'h66; // f
            8'h34: val = 8'h67; // g
            8'h33: val = 8'h68; // h
            8'h3B: val = 8'h6A; // j
            8'h42: val = 8'h6B; // k
            8'h4B: val = 8'h6C; // l
            8'h1A: val = 8'h7A; // z
            8'h22: val = 8'h78; // x
            8'h21: val = 8'h63; // c
            8'h2A: val = 8'h76; // v
            8'h32: val = 8'h62; // b
            8'h31: val = 8'h6E; // n
            8'h3A: val = 8'h6D; // m
            8'h29: val = 8'h20; // Space
            8'h0E: val = 8'h60; // `
            8'h16: val = 8'h31; // 1
            8'h1E: val = 8'h32; // 2
            8'h26: val = 8'h33; // 3
            8'h25: val = 8'h34; // 4
            8'h2E: val = 8'h35; // 5
            8'h36: val = 8'h36; // 6
            8'h3D: val = 8'h37; // 7
            8'h3E: val = 8'h38; // 8
            8'h46: val = 8'h39; // 9
            8'h45: val = 8'h30; // 0
            8'h4E: val = 8'h2D; // -
            8'h55: val = 8'h3D; // =
            8'h54: val = 8'h5B; // [
            8'h5B: val = 8'h5D; // ]
            8'h4C: val = 8'h3B; // ;
            8'h52: val = 8'h27; // '
            8'h41: val = 8'h2C; // ,
            8'h49: val = 8'h2E; // .
            8'h4A: val = 8'h2F; // /
            8'h5D: val = 8'h5C; // \
            default: val = 8'b0; 
        endcase
    end
end

endmodule



module ps2_keyboard(clk,clrn,ps2_clk,ps2_data,data,
                    ready,nextdata_n,overflow);//这里是键盘处理部分
    input clk,clrn,ps2_clk,ps2_data;
    input nextdata_n;
    output [7:0] data;
    output reg ready;
    output reg overflow;     // fifo overflow
    // internal signal, for test
    reg [9:0] buffer;        // ps2_data bits
    reg [7:0] fifo[7:0];     // data fifo
    reg [2:0] w_ptr,r_ptr;   // fifo write and read pointers
    reg [3:0] count;  // count ps2_data bits
    // detect falling edge of ps2_clk
    reg [2:0] ps2_clk_sync;

    always @(posedge clk) begin
        ps2_clk_sync <=  {ps2_clk_sync[1:0],ps2_clk};
    end

    wire sampling = ps2_clk_sync[2] & ~ps2_clk_sync[1];

    always @(posedge clk) begin
        if (clrn == 0) begin // reset
            count <= 0; w_ptr <= 0; r_ptr <= 0; overflow <= 0; ready<= 0;
        end
        else begin
            if ( ready ) begin // read to output next data
                if(nextdata_n == 1'b0) //read next data
                begin
                    r_ptr <= r_ptr + 3'b1;
                    if(w_ptr==(r_ptr+1'b1)) //empty
                        ready <= 1'b0;
                end
            end
            if (sampling) begin
              if (count == 4'd10) begin
                if ((buffer[0] == 0) &&  // start bit
                    (ps2_data)       &&  // stop bit
                    (^buffer[9:1])) begin      // odd  parity
                    fifo[w_ptr] <= buffer[8:1];  // kbd scan code
                    w_ptr <= w_ptr+3'b1;
                    ready <= 1'b1;
                    overflow <= overflow | (r_ptr == (w_ptr + 3'b1));
                end
                count <= 0;     // for next
              end else begin
                buffer[count] <= ps2_data;  // store ps2_data
                count <= count + 3'b1;
              end
            end
        end
    end
    assign data = fifo[r_ptr]; //always set output data

endmodule