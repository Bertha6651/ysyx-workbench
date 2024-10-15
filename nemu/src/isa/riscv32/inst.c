/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include "local-include/reg.h"
#include <cpu/cpu.h>
#include <cpu/ifetch.h>
#include <cpu/decode.h>

#define R(i) gpr(i)
#define XLEN 32 
#define Mr vaddr_read//return paddr_read(addr, len);
#define Mw vaddr_write//paddr_write(addr, len, data);

enum {
  TYPE_I, TYPE_U, TYPE_S,TYPE_B,TYPE_R,TYPE_J,
  TYPE_N, // none
};

#define src1R() do { *src1 = R(rs1); } while (0)
#define src2R() do { *src2 = R(rs2); } while (0)
#define immI() do { *imm = SEXT(BITS(i, 31, 20), 12); } while(0)
#define immU() do { *imm = SEXT(BITS(i, 31, 12), 20) << 12; } while(0)
#define immS() do { *imm = (SEXT(BITS(i, 31, 25), 7) << 5) | BITS(i, 11, 7); } while(0)
#define immB() do { *imm = (SEXT(BITS(i,31,31),1)<<12|BITS(i,7,7)<<11|BITS(i,30,25)<<5|BITS(i,11,8)<<1);}while(0)
#define immJ() do { *imm = (BITS(i,31,31)<<20|SEXT(BITS(i,19,12),8)<<12|SEXT(BITS(i,20,20),1)<<11|SEXT(BITS(i,30,21),10)<<1);}while(0)
/*
代码需要进行进一步的译码工作,由decode_operand
 这个函数将会根据传入的指令类型type来进行操作数的译码, 译码结果将记录到函数参数rd, src1, src2和imm中, 
它们分别代表目的操作数的寄存器号码, 两个源操作数和立即数.

  我们会发现, 类似寄存器和立即数这些操作数, 其实是非常常见的操作数类型. 
为了进一步实现操作数译码和指令译码的解耦, 我们对这些操作数的译码进行了抽象封装:

框架代码定义了src1R()和src2R()两个辅助宏, 用于寄存器的读取结果记录到相应的操作数变量中
框架代码还定义了immI等辅助宏, 用于从指令中抽取出立即数
有了这些辅助宏, 我们就可以用它们来方便地编写decode_operand()了, 例如RISC-V中I型指令的译码过程可以通过如下代码实现:

decode_operand中用到了宏BITS和SEXT, 它们均在nemu/include/macro.h中定义, 分别用于位抽取和符号扩展
decode_operand会首先统一对目标操作数进行寄存器操作数的译码, 即调用*rd = BITS(i, 11, 7), 不同的指令类型可以视情况使用rd
*/
static void decode_operand(Decode *s, int *rd, word_t *src1, word_t *src2, word_t *imm, int type) {
  uint32_t i = s->isa.inst.val;
  int rs1 = BITS(i, 19, 15);
  int rs2 = BITS(i, 24, 20);
  *rd     = BITS(i, 11, 7);
  switch (type) {
    case TYPE_I: src1R();          immI(); break;
    case TYPE_U:                   immU(); break;
    case TYPE_S: src1R(); src2R(); immS(); break;
    case TYPE_B: src1R(); src2R(); immB(); break;
    case TYPE_J:                   immJ(); break;
    case TYPE_R: src1R(); src2R();         break;
  }
  printf("rs1=%d,rs2=%d,rd=%d,imm=0x%x\n",rs1,rs2,*rd,*imm);
  printf("src1=0x%x,src2=0x%x\n",*src1,*src2);
}

/*
译码阶段结束之后, 代码将会执行模式匹配规则中指定的指令执行操作, 这部分操作会用到译码的结果, 
并通过C代码来模拟指令执行的真正行为. 例如对于auipc指令, 由于译码阶段已经把U型立即数记录到操作数imm中了, 
我们只需要通过R(rd) = s->pc + imm将立即数与当前PC值相加并写入目标寄存器中, 这样就完成了指令的执行.*/
// static int decode_exec(Decode *s) {
//   int rd = 0;
//   word_t src1 = 0, src2 = 0, imm = 0;
//   s->dnpc = s->snpc;
//   printf("s->dnpc:0x%x\n",s->dnpc);
// #define INSTPAT_INST(s) ((s)->isa.inst.val)
// #define INSTPAT_MATCH(s, name, type, ... /* execute body */ ) { 
//   decode_operand(s, &rd, &src1, &src2, &imm, concat(TYPE_, type)); 
//   __VA_ARGS__ ; 
// }

//   INSTPAT_START();
//   INSTPAT("??????? ????? ????? ??? ????? 00101 11", auipc  , U, R(rd) = s->pc + imm);
//   INSTPAT("??????? ????? ????? 100 ????? 00000 11", lbu    , I, R(rd) = Mr(src1 + imm, 1));
//   INSTPAT("??????? ????? ????? 000 ????? 01000 11", sb     , S, Mw(src1 + imm, 1, src2));

//   INSTPAT("0000000 00001 00000 000 00000 11100 11", ebreak , N, NEMUTRAP(s->pc, R(10))); // R(10) is $a0
//   INSTPAT("??????? ????? ????? ??? ????? ????? ??", inv    , N, INV(s->pc));
//   INSTPAT_END();

//   R(0) = 0; // reset $zero to 0

//   return 0;
// }

word_t ifSubOper(word_t imm)
{
word_t immEx=SEXT(imm,32);
word_t ifSub;

if((immEx & 0x80000000)!=0) ifSub=1;
else ifSub=0;
return ifSub;
}

void addiOper(word_t src1,word_t imm,int rd)
{
word_t immEx=SEXT(imm,32);
word_t ifSub=ifSubOper(imm);
printf("SRC1 :%x\n",src1);
if(ifSub==1)
{
  printf("It is Sub\n");
  word_t immSub=~immEx+1;
  printf("immSub is 0x%x\n",immSub);
  if(src1==0)R(rd)=-immSub;
  else {
   R(rd)= src1-immSub;
  }
}
else
{
  R(rd) = (src1==0)?immEx:src1+immEx;
}

}

void jump_all(word_t imm,word_t src1,int rd,Decode *s,int ifR)
{
  if(ifSubOper(SEXT(imm,32))==1)
    if(ifR==0)
      s->dnpc =s->dnpc-(~SEXT(imm,32)+1)-4;
    else s->dnpc =((src1-(~SEXT(imm,12)+1))&(~1))-4;
  else {
    if(ifR ==0) s->dnpc+=SEXT(imm,32)-4;
    else s->dnpc=((src1+SEXT(imm,12))&(~1))-4;}

  R(rd)=s->pc+4;//有困难
}

word_t noUOper(word_t noU)//有符号数转换
{
  if(ifSubOper(noU)==1)
    return -(~noU+1);
  else return noU;
}

void div_rem_u_nou(word_t src1, word_t src2, word_t rd, int ifU, int ifDiv)
{
  word_t tmp = 0;
  if (src2 == 0)
  {
    if (ifU)
    {
      if (ifDiv) R(rd) = 2 ^ (XLEN - 1);
      else       R(rd) = src1;
    }
    else
    {
      if (ifDiv) R(rd) = -1;
      else       R(rd) = src1;
    }
  }
  else
  {
    if (ifU){
      src1 = noUOper(src1);
      src2 = noUOper(src2);
      tmp = src1 / src2;
      if (tmp > (-2 ^ (32 - 1)))
      {
        if (ifDiv) R(rd) = -2 ^ (XLEN - 1);
        else       R(rd) = 0;
      }
      else
      {
        if (ifDiv) R(rd) = tmp;
        else       R(rd) = src1 % src2;
      }
    }
    else{
      tmp = src1 / src2;
      if (tmp > (-2 ^ (32 - 1))){
        if (ifDiv) R(rd) = -2 ^ (XLEN - 1);
        else       R(rd) = 0;
      }
      else
      {
        if (ifDiv) R(rd) = tmp;
        else       R(rd) = src1 % src2;
      }
    }
  }
}

void miu_u_nou(word_t src1, word_t src2, word_t rd, int ifU, int ifH)
{
  word_t src1_nou = noUOper(src1);
  word_t src2_nou = noUOper(src2);

  word_t tmp_u = src1*src2;
  word_t tmp_nou = src1_nou*src2_nou;
  word_t tmp_su = src1_nou*src2;

  if(!ifH)
  {
    R(rd)=BITS(tmp_u,XLEN-1,0);
  }
  else{
    switch (ifU){
    case 0:{
      R(rd)=BITS(SEXT(tmp_nou,XLEN*2),XLEN*2-1,XLEN);
      break;
    }
    case 1:{
      R(rd)=BITS(SEXT(tmp_u,XLEN*2),XLEN*2-1,XLEN);
      break;
    }
    case 2:{
      R(rd)=BITS(SEXT(tmp_su,XLEN*2),XLEN*2-1,XLEN);
      break;
    }
    default:
      break;
    }
  }
}
static int decode_exec(Decode *s) {
  int rd = 0;
  word_t src1 = 0, src2 = 0, imm = 0;
  s->dnpc = s->snpc;
  
  Log("s->dnpc:0x%x",s->dnpc);
#define INSTPAT_INST(s) ((s)->isa.inst.val)
#define INSTPAT_MATCH(s, name, type, ... /* execute body */ ) { \
  decode_operand(s, &rd, &src1, &src2, &imm, concat(TYPE_, type)); \
  __VA_ARGS__ ; \
}

  INSTPAT_START();
  INSTPAT("??????? ????? ????? ??? ????? 00101 11", auipc  , U, R(rd) = s->pc + imm);
  INSTPAT("??????? ????? ????? 100 ????? 00000 11", lbu    , I, R(rd) = Mr(src1 + imm, 1));
  INSTPAT("??????? ????? ????? 000 ????? 01000 11", sb     , S, Mw(src1 + imm, 1, src2));


  /*add*/
  INSTPAT("??????? ????? ????? ??? ????? 01101 11", lui     , U, R(rd)=SEXT(SEXT(imm,20)<<12,32));

  INSTPAT("??????? ????? ????? ??? ????? 11011 11", jal     , J, jump_all(imm,0,rd,s,0));//有困难
  INSTPAT("??????? ????? ????? 000 ????? 11001 11", jalr    , I, jump_all(imm,src1,rd,s,1));
  //B
  INSTPAT("??????? ????? ????? 000 ????? 11000 11", beq     , B, (src1==src2)?jump_all(imm,0,rd,s,0):0);//take the branch if registers rs1 and rs2 are equal .
  INSTPAT("??????? ????? ????? 001 ????? 11000 11", bne     , B, (src1!=src2)?jump_all(imm,0,rd,s,0):0);//take the branch if registers rs1 and rs2 are unequal 
  INSTPAT("??????? ????? ????? 100 ????? 11000 11", blt     , B, src1=noUOper(src1);src2=noUOper(src2);(src1<src2)?jump_all(imm,0,rd,s,0):0);
  INSTPAT("??????? ????? ????? 101 ????? 11000 11", bge     , B, src1=noUOper(src1);src2=noUOper(src2);(src1>src2)?jump_all(imm,0,rd,s,0):0);
  INSTPAT("??????? ????? ????? 110 ????? 11000 11", bltu    , B, (src1<src2)?jump_all(imm,0,rd,s,0):0);
  INSTPAT("??????? ????? ????? 111 ????? 11000 11", bgeu    , B, (src1>src2)?jump_all(imm,0,rd,s,0):0);
  //L
  INSTPAT("??????? ????? ????? 000 ????? 00000 11", lb      , I, R(rd) = SEXT(Mr(src1 + SEXT(imm,12), 1),32));
  INSTPAT("??????? ????? ????? 001 ????? 00000 11", lh      , I, R(rd) = SEXT(Mr(src1 + SEXT(imm,12), 2),32));//LH指令从存储器中读取一个16位数值
  INSTPAT("??????? ????? ????? 010 ????? 00000 11", lw      , I, R(rd) = Mr(src1 + SEXT(imm,12), 4));//指令将一个32位数值从存储器复制到rd中
  INSTPAT("??????? ????? ????? 100 ????? 00000 11", lbu     , I, R(rd) = Mr(src1 + SEXT(imm,12), 1));//8位数值，零扩展到32位
  INSTPAT("??????? ????? ????? 101 ????? 00000 11", lhu     , I, R(rd) = Mr(src1 + SEXT(imm,12), 2));//16位数值，零扩展到32位
  
  //S
  INSTPAT("??????? ????? ????? 000 ????? 01000 11", sb      , S, Mw(src1 + SEXT(imm,12), 1, src2));
  INSTPAT("??????? ????? ????? 001 ????? 01000 11", sh      , S, Mw(src1 + SEXT(imm,12), 2, src2));//这里还有问题
  INSTPAT("??????? ????? ????? 010 ????? 01000 11", sw      , S, Mw(src1 + SEXT(imm,12), 4, src2));
  
  //
  INSTPAT("??????? ????? ????? 000 ????? 00100 11", addi    , I, addiOper(src1,imm,rd));
  INSTPAT("??????? ????? ????? 010 ????? 00100 11", slti    , I, R(rd) = (src1<SEXT(imm,12))?1:0);
  INSTPAT("??????? ????? ????? 011 ????? 00100 11", sltiu   , I, R(rd) = (src1<SEXT(imm,32)?1:0));
  INSTPAT("??????? ????? ????? 100 ????? 00100 11", xori    , I, R(rd) = src1 ^ SEXT(imm,12));
  INSTPAT("??????? ????? ????? 110 ????? 00100 11", ori     , I, R(rd) = src1 | SEXT(imm,12));
  INSTPAT("??????? ????? ????? 111 ????? 00100 11", andi    , I, R(rd) = src1 & SEXT(imm,12));
  
  INSTPAT("0000000 ????? ????? 001 ????? 00100 11", slli    , I, R(rd) = src1<<(BITS(imm,4,0)&0x1f));//逻辑左移
  INSTPAT("0000000 ????? ????? 101 ????? 00100 11", srli    , I, R(rd) = src1>>(BITS(imm,4,0)&0x1f));//逻辑右移
  INSTPAT("0100000 ????? ????? 101 ????? 00100 11", srai    , I, R(rd) = SEXT(src1>>(BITS(imm,4,0)&0x1f),sizeof(src1)*8));//算术右移
  
  INSTPAT("0000000 ????? ????? 000 ????? 01100 11", add      , R, R(rd) = src1+src2);//+
  INSTPAT("0100000 ????? ????? 000 ????? 01100 11", sub      , R, R(rd) = src1-src2);//-
  INSTPAT("0000000 ????? ????? 001 ????? 01100 11", sll      , R, R(rd) = src1<<(src2&0x1f));//逻辑左移
  INSTPAT("0000000 ????? ????? 010 ????? 01100 11", slt      , R, R(rd) = (src1<src2)?1:0);//符号数比较
  INSTPAT("0000000 ????? ????? 011 ????? 01100 11", sltu     , R, R(rd) = (SEXT(src1,32)<SEXT(src2,32))?1:0);//无符号数比较
  INSTPAT("0000000 ????? ????? 100 ????? 01100 11", xor      , R, R(rd) = src1^src2);//^
  INSTPAT("0000000 ????? ????? 101 ????? 01100 11", srl      , R, R(rd) = src1>>(src2&0x1f));//逻辑右移
  INSTPAT("0100000 ????? ????? 101 ????? 01100 11", sra      , R, R(rd) = SEXT(src1>>(BITS(imm,4,0)&0x1f),sizeof(src1)*8));//算术右移
  INSTPAT("0000000 ????? ????? 110 ????? 01100 11", or       , R, R(rd) = src1|src2);//|
  INSTPAT("0000000 ????? ????? 111 ????? 01100 11", and      , R, R(rd) = src1&src2);//&

  //RV32M
  INSTPAT("0000001 ????? ????? 000 ????? 01100 11", mul      , R, miu_u_nou(src1,src2,rd,0,0)) ; //乘法，储存低位
  INSTPAT("0000001 ????? ????? 001 ????? 01100 11", mulh     , R, miu_u_nou(src1,src2,rd,0,1)) ; //乘法，储存高位，有符号x有符号（nou）
  INSTPAT("0000001 ????? ????? 010 ????? 01100 11", mulhsu   , R, miu_u_nou(src1,src2,rd,1,1)) ; //乘法，储存高位，无符号x无符号（u）
  INSTPAT("0000001 ????? ????? 011 ????? 01100 11", mulhu    , R, miu_u_nou(src1,src2,rd,2,1)) ; //乘法，储存高位，有符号x无符号（su）
  INSTPAT("0000001 ????? ????? 100 ????? 01100 11", div      , R, div_rem_u_nou(src1,src2,rd,0,1)) ; //&
  INSTPAT("0000001 ????? ????? 101 ????? 01100 11", divu     , R, div_rem_u_nou(src1,src2,rd,1,1)) ; //&
  INSTPAT("0000001 ????? ????? 110 ????? 01100 11", rem      , R, div_rem_u_nou(src1,src2,rd,0,0)) ; //&
  INSTPAT("0000001 ????? ????? 111 ????? 01100 11", remu     , R, div_rem_u_nou(src1,src2,rd,1,0)) ; //&
  
  INSTPAT("00000?? ????? 00000 000 00000 00011 11", fence    , S, Mw(src1 + imm, 1, src2));
  INSTPAT("0000000 00000 00000 001 00000 00011 11", fence_i  , S, Mw(src1 + imm, 1, src2));
  
  INSTPAT("0000000 00000 00000 000 00000 11100 11", ecall    , N, Mw(src1 + imm, 1, src2));
  INSTPAT("0000000 00001 00000 000 00000 11100 11", ebreak   , N, NEMUTRAP(s->pc, R(10))); // R(10) is $a0
  
  // INSTPAT("??????? ????? ????? 001 ????? 11100 11", csrrw    , S, Mw(src1 + imm, 1, src2));
  // INSTPAT("??????? ????? ????? 010 ????? 11100 11", csrrs    , S, Mw(src1 + imm, 1, src2));
  // INSTPAT("??????? ????? ????? 011 ????? 11100 11", csrrc    , S, Mw(src1 + imm, 1, src2));
  // INSTPAT("??????? ????? ????? 101 ????? 11100 11", csrrwi   , S, Mw(src1 + imm, 1, src2));
  // INSTPAT("??????? ????? ????? 110 ????? 11100 11", csrrsi   , S, Mw(src1 + imm, 1, src2));
  // INSTPAT("??????? ????? ????? 111 ????? 11100 11", csrrci   , S, Mw(src1 + imm, 1, src2));

  INSTPAT("??????? ????? ????? ??? ????? ????? ??", inv    , N, INV(s->pc));

  INSTPAT_END();

  R(0) = 0; // reset $zero to 0

  return 0;
}

int isa_exec_once(Decode *s) {
  s->isa.inst.val = inst_fetch(&s->snpc, 4);
  printf("s->isa.inst.val is 0x%x\n",s->isa.inst.val);
  return decode_exec(s);
}
