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

#include "sdb.h"

#define NR_WP 32

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;//个指向下一个监视点的指针(链表)

  /* TODO: Add more members if necessary */

} WP;

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

void init_wp_pool() 
{
  int i;
  for (i = 0; i < NR_WP; i ++) {
  // #define NR_WP 32
    wp_pool[i].NO = i;//初始化编号
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);//如果i=31，下一个指令为空，否则指向wp_pool[i-1],链表结构
  }

  head = NULL;
  free_ = wp_pool;//指向监视池开头，准备接受新的监视点请求。
}

/* TODO: Implement the functionality of watchpoint */

