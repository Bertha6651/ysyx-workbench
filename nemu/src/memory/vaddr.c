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
#include <memory/paddr.h>

word_t vaddr_ifetch(vaddr_t addr, int len) {
  return paddr_read(addr, len);
}

word_t vaddr_read(vaddr_t addr, int len) {
  return paddr_read(addr, len);
}

void vaddr_write(vaddr_t addr, int len, word_t data) {
  paddr_write(addr, len, data);
}

uint32_t memory_scan_one(uint32_t num_addr,int bytes_per_line)
{
  return vaddr_read(num_addr,bytes_per_line);
}

void memory_scan(uint32_t start_addr,int length)
{
  int bytes_per_line =4;
  for(int i=0;i<length*bytes_per_line;i+=bytes_per_line)
  {
    uint32_t num_addr=start_addr+i;
  printf("addr:0x%08x\t",num_addr);
  uint32_t data = memory_scan_one(num_addr,bytes_per_line);
  printf("data:%08x\n",data);

  }
}