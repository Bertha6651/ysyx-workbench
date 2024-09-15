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

#include <isa.h>
#include "local-include/reg.h"

const char *regs[] = {
  "$0", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
  "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
  "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
  "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
};

void isa_reg_display() {
    for (int i = 0; i < 32; i++) 
    {
    /* 打印寄存器名称和内容 */
    printf("id_%d:reg_name %s\t%x\n", i, reg_name(i), gpr(i));
    if(!(i+1)%4)  printf("\n");
    }
    /* pc 寄存器 */
    printf("id_%d:reg_name %s\t%x\n", 33, "pc", cpu.pc);
}

int isa_reg_num(const char*s)
{
    for (int i = 0; i < 32; i++) {
        if (strcmp(s, regs[i]) == 0) {
            return i ;  // 返回对应编号
        }
    }
    if (strcmp(s, "pc") == 0) 
    {
        return 33;
    }
    return -1;
}
word_t isa_reg_str2val(const char *s, bool *success) {
    s+=1;
    // 遍历所有寄存器名进行匹配
    int num=isa_reg_num(s);
    if(num!=-1 && num !=33)//是否匹配寄存器成功
    {
        return gpr(num);
    }


    // 特殊处理 pc 寄存器
    if (strcmp(s, "pc") == 0) {
        *success = true;
        return cpu.pc;  // 返回 pc 寄存器的值
    }

    // 未找到匹配的寄存器
    *success = false;
    printf("Error: Unknown register %s\n", s);
    return 0;
}

