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

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>
#include <stdbool.h>
#include "expr.h"

enum
{
  TK_NOTYPE = 256,
  TK_EQ,     // ==
  TK_NUM,    // 数字
  TK_HEX,    // 十六进制整数
  TK_ID,     // 标识符
  TK_REG,    // 寄存器，如$a0
  TK_DEREF,  // 指针解引用 *
  TK_PLUS,   // +
  TK_MINUS,  // -
  TK_MUL,    // *
  TK_DIV,    // /
  TK_LPAREN, // (
  TK_RPAREN, // )
  /* TODO: Add more token types */

};

static struct rule
{
  const char *regex;
  int token_type;
} rules[] = {

    /* TODO: Add more rules.
     * Pay attention to the precedence level of different rules.
     */

    {" +", TK_NOTYPE}, // spaces
    {"\\+", TK_PLUS},  // plus
    {"==", TK_EQ},     // equal

    /*my add rules*/
    {"-", TK_MINUS},                   // -
    {"\\*", TK_MUL},                   // multiply or dereference (to be handled in parsing)
    {"/", TK_DIV},                     //  /
    {"\\(", TK_LPAREN},                // (
    {"\\)", TK_RPAREN},                // )
    {"0[xX][0-9a-fA-F]+", TK_HEX},     // 16进制
    {"[0-9]+", TK_NUM},                // 数字
    {"\\$[a-zA-Z][0-9]", TK_REG},      // 寄存器，如 $a0, $t1, etc.
    {"[a-zA-Z_][a-zA-Z0-9_]*", TK_ID}, // 标识符如 `number`)
};
#define ARRLEN(arr) (int)(sizeof(arr) / sizeof(arr[0]))
#define NR_REGEX ARRLEN(rules)

// 求的是rules的条数

static regex_t re[NR_REGEX] = {};
// typedef struct re_pattern_buffer regex_t;

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
/*C语言中使用正则表达式一般分为三步：
      编译正则表达式 regcomp()：这个函数把指定的正则表达式pattern编译成一种特定的数据格式compiled
      匹配正则表达式 regexec()：使用这个数据在目标文本串中进行模式匹配。
      释放正则表达式 regfree():可以用这个函数清空regex_t结构体的内容　

    int regcomp (regex_t *compiled, const char *pattern, int cflags)

  这个函数把指定的正则表达式pattern编译成一种特定的数据格式compiled，这样可以使匹配更有效。函数regexec 会使用这个数据在目标文本串中进行模式匹配。执行成功返回０。 　

参数说明：
①regex_t 是一个结构体数据类型，用来存放编译后的正则表达式，它的成员re_nsub 用来存储正则表达式中的子正则表达式的个数，子正则表达式就是用圆括号包起来的部分表达式。
②pattern 是指向我们写好的正则表达式的指针。
③cflags 有如下4个值或者是它们或运算(|)后的值：
  REG_EXTENDED 以功能更加强大的扩展正则表达式的方式进行匹配。
  REG_ICASE 匹配字母时忽略大小写。
  REG_NOSUB 不用存储匹配后的结果。
  REG_NEWLINE 识别换行符，这样’$’就可以从行尾开始匹配，’^’就可以从行的开头开始匹配。

　    int regexec(const regex_t *preg, const char *string, size_t nmatch,regmatch_t pmatch[], int eflags);

regmatch_t 是一个结构体数据类型，在regex.h中定义：
typedef struct {
  regoff_t rm_so;//存放匹配文本串在目标串中的开始位置
  regoff_t rm_eo;//rm_eo 存放结束位置
} regmatch_t;

　　参数说明：　
　　  preg    是已经用regcomp函数编译好的正则表达式。
　  　string      是目标文本串。
　　  nmatch      是regmatch_t结构体数组的长度。
　　  matchptr    regmatch_t类型的结构体数组，存放匹配文本串的位置信息。
　　  eflags 有两个值:
      REG_NOTBOL 让特殊字符^无作用
      REG_NOTEOL 让特殊字符＄无作用

      size_t regerror (int errcode, regex_t *compiled, char *buffer, size_t length)

当执行regcomp 或者regexec 产生错误的时候，就可以调用这个函数而返回一个包含错误信息的字符串。
errcode 是由regcomp 和 regexec 函数返回的错误代号。
compiled 是已经用regcomp函数编译好的规则表达式，这个值可以为NULL。
buffer 指向用来存放错误信息的字符串的内存空间。
length 指明buffer的长度，如果这个错误信息的长度大于这个值，则regerror 函数会自动截断超出的字符串，
但他仍然会返回完整的字符串的长度。所以我们可以用如下的方法先得到错误字符串的长度。
*/
void init_regex() // 初始化正则表达式（regex）并编译,初始化的就是rules[i].regex，到&re[i]结构体中
{
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i++)
  {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED); // 编译正则表达式
    if (ret != 0)
    {
      regerror(ret, &re[i], error_msg, 128);
      printf("regex compilation failed: %s\n%s", error_msg, rules[i].regex); 
    }
  }
}

typedef struct token
{
  int type;     // 用于记录token的类型.
  char str[32]; // 记录token相应的子串，如十进制数的值
} Token;

static Token tokens[32] __attribute__((used)) = {}; // tokens数组用于按顺序存放已经被识别出的token信息
static int nr_token __attribute__((used)) = 0;      // 指示已经被识别出的token数目

static bool make_token(char *e)
{
  int position = 0;
  int i;
  regmatch_t pmatch; // 这个结构体主要是用来储存匹配文本串在目标串中的开始位置和存放结束位置

  nr_token = 0; // 显示已经识别出的token数目

  while (e[position] != '\0')
  {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i++) // 按照顺序匹配rules
    {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) // rm_so是开始的位置
      {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo; // 开始位置rm_so是0，这自然结束位置就是这段长度了

        printf("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);
        // rules号，rules的情况   ，到字符的位置，匹配长度    ，匹配长度   ，匹配的字符
        position += substr_len; // 指针开始移动到下一个匹配字符的首部了

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type)
        {
        case TK_NOTYPE:
          break; // 跳过空白字符
        case TK_NUM:
        case TK_HEX:
        case TK_REG:
        case TK_ID:
          // 将匹配到的字符串存储到token的str字段中
          strncpy(tokens[nr_token].str, substr_start, substr_len);
          tokens[nr_token].str[substr_len] = '\0';
          tokens[nr_token].type = rules[i].token_type;
          nr_token++;
          break;

        case TK_PLUS:
        case TK_MINUS:
        case TK_MUL:
        case TK_DIV:
        case TK_EQ:
        case TK_LPAREN:
        case TK_RPAREN:
          // 对于这些运算符，直接记录类型
          tokens[nr_token].type = rules[i].token_type;
          nr_token++;
          break;

        case TK_DEREF:
          // 检查前一个token以确定`*`是解引用还是乘法
          if (nr_token == 0 || tokens[nr_token - 1].type == TK_PLUS ||
              tokens[nr_token - 1].type == TK_MINUS || tokens[nr_token - 1].type == TK_MUL ||
              tokens[nr_token - 1].type == TK_DIV || tokens[nr_token - 1].type == TK_LPAREN)
          {
            tokens[nr_token].type = TK_DEREF; // 解引用
          }
          else
          {
            tokens[nr_token].type = TK_MUL; // 乘法
          }
          nr_token++;
          break;

        default:
          TODO();
        }

        break;
      }
    }

    if (i == NR_REGEX)
    {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

bool check_parentheses(int p, int q)
{
  // Log("check_parentheses   p:%d\tq:%d\n",p,q);
  // Log("tokens[p].type:%d\tTK_LPAREN:%d\n",tokens[p].type,TK_LPAREN);
  // Log("tokens[q].type:%d\tTK_RPAREN:%d\n",tokens[q].type,TK_RPAREN);

  if (tokens[p].type == TK_LPAREN && tokens[q].type == TK_RPAREN)
  {
    int count = 0;
    for (int i = p; i <= q; i++)
    {
      if (tokens[i].type == TK_LPAREN)
      {
        count++;
        // Log("count:%d\n",count);
      }
      else if (tokens[i].type == TK_RPAREN)
      {
        count--;
        // Log("count:%d\n",count);
      }

      if (count == 0 && i < q)
      {
        return false;
      } // 说明括号不匹配
      // 这一条匹配了第四五种情况，琢磨了很久才明白
    }
    return (count == 0); // true 表示完全匹配
  }
  else
  {
    return false; // 没有被括号包围
  }
}

int get_operator_priority(int op) // 这个函数是用来得到优先级排名的，
{
  switch (op)
  {
  case TK_PLUS:
  case TK_MINUS:
    return 1; // 加减法优先级比较低
  case TK_MUL:
  case TK_DIV:
    return 2; // 乘除法优先级要大一些
  default:
    return 100; // 假设一个比所有运算符优先级高的值
  }
}

int find_main_operator(int p, int q)
{
  int main_op = -1;
  int min_priority = 100;
  int count = 0; // 用count来判断是否在括号内
  for (int i = p; i <= q; i++)
  {
    if (tokens[i].type == TK_LPAREN)
    {
      count++;
      // Log("count:%d\n",count);
    }
    else if (tokens[i].type == TK_RPAREN)
    {
      count--;
      // Log("count:%d\n",count);
    }
    else if (count == 0) // 这就是在括号外的意思了
    {
      int he_priority = get_operator_priority(tokens[i].type);
      if (he_priority <= min_priority)
      {
        min_priority = he_priority;
        main_op = i;
      }
    }
  }
  return main_op;
}

uint32_t eval(int p, int q, bool *success)
{
  // printf("p:%d\tq:%d\n",p,q);
  if (p > q)
  {
    assert(0);
  }
  else if (p == q) // 应该是个数字
  {
    return strtol(tokens[p].str, NULL, 10); // 返回数字的值，这里注意字符的转换
  }
  else if (check_parentheses(p, q) == true) // 括号匹配处理
  {
    Log("check_parenthese success");
    return eval(p + 1, q - 1, success);
  }
  else // 这里主要是处理更复杂的情况，比如说第三种、第五种
  {
    int op = find_main_operator(p, q);
    Log("op:%d\t tokens[op].type:%d", op, tokens[op].type);
    uint32_t val1 = eval(p, op - 1, success);
    printf("val1:%d\n", val1);
    uint32_t val2 = eval(op + 1, q, success);
    if (tokens[op].type == TK_DIV && val2 == 0)
    {
      Log("0 cannot be used as a divisor, please re-enter."); // 除0预防
      *success = false;
      return 0;
    }
    printf("val2:%d\n", val2);

    switch (tokens[op].type)
    {
    case TK_PLUS:
      return val1 + val2;
    case TK_MINUS:
      return val1 - val2;
    case TK_MUL:
      return val1 * val2;
    case TK_DIV:
      return val1 / val2;
    default:
      assert(0);
    }
  }
}

int expr(char *e, bool *success)
{
  if (!make_token(e))
  {
    *success = false;
    printf("make_token false\n");
    return 0;
  }

  printf("%d\n", nr_token);
  uint32_t result = eval(0, nr_token - 1, success);
  /* TODO: Insert codes to evaluate the expression. */
  // TODO();
  if (*success)
  {
    Log("result:%d", result);
    return result;
  }
  else
  {
    printf("error\n");
    return 0;
  }
}

// void gen_rand_expr() //生成测试用的表达式
// {
//   switch (choose(3)) {
//     case 0: gen_num(); break;
//     case 1: gen('('); gen_rand_expr(); gen(')'); break;
//     default: gen_rand_expr(); gen_rand_op(); gen_rand_expr(); break;
//   }
// }