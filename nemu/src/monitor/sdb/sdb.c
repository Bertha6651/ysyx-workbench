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
#include <memory/vaddr.h>
#include <memory.h>
#include <cpu/cpu.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <stdio.h>  
#include <stdlib.h>  
#include "sdb.h"
#include <utils.h>



static int is_batch_mode = false;

//初始化正则表达式
void init_regex();

//监视功能函数
void init_wp_pool();
void wp_pool_display();
void set_wp(char *args);
void display_watchPoints();
void free_wp(int num);
bool check_watchPoints(); //检查监视点的值是否改变



/* We use the `readline' library to provide more flexibility to read from stdin. */
/* readline():Read a line of input.  Prompt with PROMPT.  A NULL PROMPT means none. 
readline() 的参数是一个字符串，调用函数的时候会在屏幕上输出，这个函数会读取一行输入，然后返回一个指向输入字符串的指针，
readline 会为输入的字符串动态分配内存，所以使用完之后需要free掉。
*/
static char* rl_gets() 
{
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");//readline - get a line from a user with editing

  if (line_read && *line_read) 
  {
    add_history(line_read);//add history
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}


static int cmd_q(char *args) 
{
  nemu_state.state=NEMU_QUIT;//8_25 退出时记得要改变状态
  return -1;
}

static int cmd_help(char *args);

static int cmd_si(char *args){
  int Num;
  if (args == NULL){
    cpu_exec(1);
  }else {
      int n = sscanf(args, "%d", &Num);  
       if (n == 1) {  
        // printf("successfully obtained the integer: %d\n", Num);  
        cpu_exec(Num);
      } else {  
        printf("Incorrect input format, please enter again. \tReference format: si [N]");  
        return 0;
      }  
      return 0;
  }
  return 0;
}



static int cmd_info(char *args){
  if(args==NULL)
  {
    printf("Incorrect input format, please enter again.\n");  
    printf("Reference format: info SUBCMD\n\tinfo r: Print the state of registers\n\tinfo w: watchpoint information.\n");  
    return 0;  
  }
  else if (strcmp(args, "r") == 0){
    isa_reg_display();
    return 0;
  } 
  else if (strcmp(args,"w")==0){
  display_watchPoints();
    return 0;

  }
  else{
    printf("Incorrect input format, please enter again.\n");  
    printf("Reference format: info SUBCMD\n\tinfo r: Print the state of registers\n\tinfo w: watchpoint information.\n");  
    return 0;  
  }
  return 0;
}

static int cmd_x(char *args){
  char *N = strtok(NULL, " ");
  char *EXPR = strtok(NULL, " ");

  if (N == NULL || EXPR == NULL) {
        printf("Usage: x <length> <address>\n");
        return 0;
  }

  int Num;
  int n = sscanf(N, "%d", &Num);//获取第二个字段，并转换为int型
  if (n != 1) {  
    printf("Incorrect input format, please enter again. \tReference format: si [N]");  
    return 0;
  }   

  // printf("N:%d\n",Num);
  // printf("EXPR:%s\n",EXPR);

  bool variable = true;  // 布尔变量
  bool* ifSuccess = &variable;  // 布尔指针指向布尔变量
  
  uint32_t start_addr =  expr(EXPR, ifSuccess);
  if(!ifSuccess)
  {
    printf("表达式错误或者程序错误\n");
  }

  memory_scan(start_addr, Num);  // 执行内存扫描
return 0;
}

static int cmd_p(char *args){
  char *EXPR=args;
  // printf("%s\n",EXPR);
  //这里之前遇到一点bug
  bool variable = true;  // 布尔变量
  bool* ifSuccess = &variable;  // 布尔指针指向布尔变量
  
  expr(EXPR, ifSuccess);

  if(!ifSuccess)
  {
    printf("表达式错误或者程序错误\n");
  }
  return 0;
}

static int cmd_w(char *args){
  if(args==NULL)
  {
    printf("Incorrect input format, please enter again.\n");  
    return 0;  
  }
set_wp(args);


return 0;
}

static int cmd_d(char *args){
  if(args==NULL)
  {
    printf("Incorrect input format, please enter again.\n");  
    return 0;  
  }
  int Num;
  int n = sscanf(args, "%d", &Num);//获取第二个字段，并转换为int型
  if (n != 1) {  
    printf("Incorrect input format, please enter again. \tReference format: d [N]");  
    return 0;
  }
 free_wp( Num);

return 0;
}

static struct {  
  const char *name;  
  const char *description;  
  int (*handler) (char *);  //这是一个指向函数的指针，函数返回类型为 int，接受一个 char * 类型的参数。
} cmd_table [] = {  
  { "help", "Display information about all supported commands", cmd_help },  
  { "c", "Continue the execution of the paused program", cmd_c },  
  { "q", "Exit NEMU", cmd_q },  
  { "si", "Execute the program one step at a time for N instructions,pausing after, \n     defaulting to 1 if N is not provided.", cmd_si },  
  { "info", "Print the state of registers and watchpoint information", cmd_info }, // Assuming cmd_info exists  
  { "x", "Evaluate the expression EXPR to get the starting memory address and \n    output N consecutive 4-byte values in hexadecimal.", cmd_x }, // Assuming cmd_x exists  
  { "p", "Calculate the value of the expression EXPR; \n    supported operations are described in the expression evaluation section of debugging.", cmd_p }, // Assuming cmd_p exists  
  { "w", "Pause program execution when the value of the expression EXPR changes.", cmd_w }, // Assuming cmd_w exists  
  { "d", "Delete the watchpoint with index N.", cmd_d } // Assuming cmd_d exists  
  /* TODO: Add more commands if necessary */  
};  


#define NR_CMD ARRLEN(cmd_table)//#define ARRLEN(arr) (int)(sizeof(arr) / sizeof(arr[0]))cmd_table的个数

static int cmd_help(char *args) 
{
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void sdb_set_batch_mode() {
  is_batch_mode = true;
}

void sdb_mainloop() 
{
  if (is_batch_mode) //batch：批处理
  {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL; ) //rl_gets获取输入的一行，直到获取完所有行
  {
    char *str_end = str + strlen(str);//将这个指针指到最后

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
      /*strtok, strtok_r - extract tokens from strings  分解字符串 str 为一组字符串，delim 为分隔符。
        根据空格获取指令
           The  strtok()  function  breaks  a string into a sequence of zero or more
         nonempty tokens.  On the first call to strtok(), the string to be  parsed
         should  be  specified  in str.  In each subsequent call that should parse
         the same string, str must be NULL.*/   

    if (cmd == NULL) { continue; }//如果cmd直接等于NULL，继续循环

    /* treat the remaining string as the arguments,将剩下的string当做参数
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;//获取后面的参数（如help w ,获取了w）
    if (args >= str_end)
    {
      args = NULL;
    }

#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) 
    {
      if (strcmp(cmd, cmd_table[i].name) == 0) //匹配cmd_table
      /*C 库函数 int strcmp(const char *str1, const char *str2) 
      把 str1 所指向的字符串和 str2 所指向的字符串进行比较。

      如果返回值小于 0，则表示 str1 小于 str2。
      如果返回值大于 0，则表示 str1 大于 str2。
      如果返回值等于 0，则表示 str1 等于 str2。*/
      {
        if (cmd_table[i].handler(args) < 0) 
        { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}

void init_sdb() {
  /* Compile the regular expressions. */
  init_regex();////初始化正则表达式（regex）并编译

  /* Initialize the watchpoint pool. */
  init_wp_pool();//初始化一个监视点池
}

void wp_pool_display()
{
  
}