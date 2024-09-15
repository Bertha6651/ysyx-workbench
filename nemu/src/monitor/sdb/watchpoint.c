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
#include <isa.h>
#include <regex.h>

#define NR_WP 32

typedef struct watchpoint
{
  int NO;
  struct watchpoint *next; // 个指向下一个监视点的指针(链表)

  //新添加的值
  char expr[256];  // 监视的表达式
  uint32_t value;
  /* TODO: Add more members if necessary */

} WP;

static WP wp_pool[NR_WP] = {};         // 定义了监视点结构的池wp_pool
static WP *head = NULL, *free_ = NULL; // 其中head用于组织使用中的监视点结构, free_用于组织空闲的监视点结构

void init_wp_pool() // 函数会对两个链表进行初始化
{
  int i;
  for (i = 0; i < NR_WP; i++)
  {
    // #define NR_WP 32
    wp_pool[i].NO = i;                                           // 初始化编号
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]); // 如果i=31，下一个指令为空，否则指向wp_pool[i-1],链表结构
    wp_pool[i].value=0;
  }

  head = NULL;
  free_ = wp_pool; // 指向监视池开头，准备接受新的监视点请求。
}

/* TODO: Implement the functionality of watchpoint */


WP *new_wp(char *args)
{
  if (free_ == NULL) // 没有空闲的监视点了
  {
    assert(0); // 终止程序
  }

  // 从free_链表中取出一个监视点
  WP *wp = free_;
  free_ = free_->next;//指向下一个

  

  strncpy(wp->expr, args, sizeof(wp->expr) - 1);  
wp->expr[sizeof(wp->expr) - 1] = '\0'; // 确保字符串以 '\0' 结尾
  // Optionally, initialize to empty string  
  
  bool variable = true;  // 布尔变量
  bool* ifSuccess = &variable;  // 布尔指针指向布尔变量
  
  wp->value=expr(wp->expr,ifSuccess);
  // 返回新的监视点
  // 插入到head链表的头部

  wp->next = head;
  head = wp;
  
  return wp;
}

void free_wp(int num)
{
  if (head == NULL) 
  {  
        printf("No watchPoints to delete.\n");  
        return; // Check if the list is empty  
  }  

  WP *wp=NULL;
  // 从head链表中移除wp
  if (head->NO == num) {
    wp=head;
    head = head->next;//直接在头部
    printf("the delete wp No is %d\n",wp->NO);
  // 将wp插入到free_链表的头部
    wp->next = free_;
    free_ = wp;
  } 
  else {//在链表中间
    WP *prev = head;

    
    while (prev->next != NULL && prev->next->NO != num) {
      prev = prev->next;
    }
    wp=prev->next;
    printf("the delete wp No is %d\n",wp->NO);
    
    //移除操作
    prev->next=wp->next;
    wp->next=free_;
    free_=wp;
  }
  
}

void display_watchPoints() {
  WP *wp = head;
  printf("Here are the used watchPoints:\n");
  while (wp != NULL) {
    printf("WatchPoint No:%d\t expr:%s\t value:%u\n", wp->NO, wp->expr,wp->value);
    wp = wp->next;
  }
}

int find_wp(char *expr)
{
  if(head ==NULL)
  {
    return -1;
  }

  WP *wp=head;
  while(strstr(wp->expr,expr)==NULL && wp !=NULL)
  {
    wp=wp->next;
  }
  
  if(wp==NULL)
  {
    return -1;
  }

  return wp->NO;
}

bool check_watchPoints(int Num) //检查监视点的值是否改变
{
  if(Num == -1)//如果Num=-1，直接返回-1；
  {
    return false;
  }
  WP *wp = NULL;
  if (head->NO == Num) {
    wp=head;
  } 
  else {//在链表中间
    WP *prev = head;

    
    while (prev->next != NULL && prev->next->NO != Num) {
      prev = prev->next;
    }
    wp=prev->next;
  }

  bool changed = false;

  while (wp != NULL) {

  bool variable = true;  // 布尔变量
  bool* ifSuccess = &variable;  // 布尔指针指向布尔变量

  uint32_t new_value = expr(wp->expr, ifSuccess);  

    if (new_value != wp->value) {
      printf("WatchPoint %d: %s has changed from %u to %u\n", wp->NO, wp->expr, wp->value, new_value);
      Log("the program stops.");
      
      wp->value = new_value;
      changed = true;
    }

    wp = wp->next;
  }

  return changed;
}

int check_watchpoint_all()
{
  WP *p=head;
  for(;p;p=p->next)
  {
    bool success=true;
    uint32_t val=expr(p->expr,&success);
    if(success == false) assert(0);
    if(val !=p->value)
    {
      p->value=val;
      return p->NO;
    }
  }
  return -1;
}

void set_wp(char *args)
{
 new_wp(args);
  printf("WatchPoint No:%d\t expr:%s\t value:%u\n", head->NO, head->expr,head->value);
}