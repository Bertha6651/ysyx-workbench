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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include "expr.h"
// #include <include/utils.h>
// #include <isa.h>
// #include <cpu/cpu.h>
// #include <readline/readline.h>
// #include <readline/history.h>


// this should be enough
static char buf[65536] = {};
static char code_buf[65536 + 128] = {}; // a little larger than `buf`
static char *code_format =
    "#include <stdio.h>\n"
    "int main() { "
    "  unsigned result = %s; "
    "  printf(\"%%u\", result); "
    "  return 0; "
    "}";

uint32_t choose(uint32_t n)
{
  if (n == 0)
  {
    return 0; // 如果 n 为 0，返回 0
  }
  return rand() % n; // 生成一个小于 n 的随机数
}
static void check_div_zero(char *exp)
{
  char *pos = strtok(exp, "/");
  if (pos != NULL)
  {
    printf("there is zero /\n");
  }
  for (; pos == NULL; pos = strtok(NULL, "/"))
  {
    const char *start = pos;
    start++;
    const char *end = start;
    int count = 0;
    // 逐字符检查，直到遇到下一个运算符
    while (*end)
    {
      if (*end == '(')
      {
        count++; // 进入括号
      }
      else if (*end == ')')
      {
        count--; // 退出括号
      }
      else if ((strchr("+-*/", *end) != NULL) && count == 0)
      {
        break; // 遇到运算符并且不在括号内，停止
      }
      end++;
    }
    // 提取从start到end之间的字符串
    size_t length = end - start;

    // printf("the length before '/' is %d\n", length);
    char *beforeStr = (char *)malloc(length + 1);
    strncpy(beforeStr, start, length);
    beforeStr[length] = '\0';

  bool variable = true;  // 布尔变量
  bool* ifSuccess = &variable;  // 布尔指针指向布尔变量
  int result= expr(beforeStr, ifSuccess);
  
    if (result == 0)
    {
      char *temp = pos + length + 1;
      memmove(pos, temp, strlen(temp) + 1);
    }
  }
}

static void gen_num()
{
  int length = strlen(buf);
  int num = choose(20);
  if (length > 0)
  {
    char last_char = buf[length - 1]; // 获取最后一位字符
    if (last_char == '/')
      num++;
  }
  char temp[20];
  sprintf(temp, "%d", num);
  strcat(buf, temp);
}

static void gen_rand_space()
{
  switch (choose(2))
  {
  case 0:
    int loop = choose(5);
    for (int i = 0; i <= loop; i++) // 不定长度的空格
    {
      strcat(buf, " ");
    }
    break;
  default:
    break;
  }
}
static void gen_rand_op()
{
  gen_rand_space();
  switch (choose(4))
  {
  case 0:
    strcat(buf, "+");
    break;
  case 1:
    strcat(buf, "-");
    break;
  case 2:
    strcat(buf, "*");
    break;
  case 3:
    strcat(buf, "/");
    break;
  default:
    break;
  }
  gen_rand_space();
}

static void gen(char a)
{
  char temp[20];
  sprintf(temp, "%c", a);
  strcat(buf, temp);
}
/*bool variable = true;  // 布尔变量
  bool* ifSuccess = &variable;  // 布尔指针指向布尔变量
    expr(EXPR, ifSuccess);

  if(!ifSuccess)
  {
    printf("表达式错误或者程序错误\n");
  }
  return 0;*/
static void gen_rand_expr()
{
  int length = strlen(buf);

  switch (choose(3))
  {
  case 0:
    if (length > 0)
    {
      char last_char = buf[length - 1]; // 获取最后一位字符
      if (last_char == ')')
        gen_rand_op();
    }
    gen_num();
    break;
  case 1:
    if (length > 0)
    {
      char last_char = buf[length - 1]; // 获取最后一位字符
      if (last_char == ')')
        gen_rand_op();
      else if (last_char >= '0' && last_char <= '9')
        gen_rand_op();
    }

    gen('(');
    gen_rand_expr();
    gen(')');
    break;
  default:
    gen_rand_expr();

    gen_rand_op();

    gen_rand_expr();
    break;
  }

  strcat(buf, "\0");
}

int main(int argc, char *argv[])
{
  int seed = time(0);
  srand(seed);
  int loop = 1;
  if (argc > 1)
  {
    sscanf(argv[1], "%d", &loop); /*
     int sscanf(const char *str, const char *format, ...)
     str -- 这是 C 字符串，是函数检索数据的源。
     format -- 这是 C 字符串，包含了以下各项中的一个或多个：空格字符、非空格字符 和 format 说明符。

     这里是读取argv[1]中的整数，读取到的在loop中*/
  }
  int i;
  for (i = 0; i < loop; i++)
  {
    gen_rand_expr();
    check_div_zero(buf);
    sprintf(code_buf, code_format, buf); // 把code_format(buf)保存到code_buf中

    FILE *fp = fopen("/tmp/.code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp); // 向文件写入code_buf
    fclose(fp);

    int ret = system("gcc /tmp/.code.c -o /tmp/.expr"); // 编译.code.c为.expr
    if (ret != 0)
      continue;

    fp = popen("/tmp/.expr", "r");
    /*popen()会调用fork()产生子进程，然后从子进程中调用/bin/sh -c来执行参数command的指令。参数type可使用“r”代表读取，“w”代表写入。依照此type值，popen()会建立管道连到子进程的标准输出设备或标准输入设备，然后返回一个文件指针。随后进程便可利用此文件指针来读取子进程的输出设备或是写入到子进程的标准输入设备中。此外，所有使用文件指针(FILE*)操作的函数也都可以使用，除了fclose()以外。

          如果 type 为 r，那么调用进程读进 command 的标准输出。
          如果 type 为 w，那么调用进程写到 command 的标准输入。*/

    assert(fp != NULL);

    int result;
    ret = fscanf(fp, "%d", &result);
    pclose(fp);

    printf("%u %s\n", result, buf);
  }
  return 0;
}
