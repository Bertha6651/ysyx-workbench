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

#include <cpu/cpu.h>
#include <cpu/decode.h>
#include <cpu/difftest.h>
#include <locale.h>

// bool check_watchPoints(int Num); //检查监视点的值是否改变
int find_wp(char *expr);
int check_watchpoint_all();

/* The assembly code of instructions executed is only output to the screen
 * when the number of instructions executed is less than this value.
 * This is useful when you use the `si' command.
 * You can modify this value as you want.
 */
#define MAX_INST_TO_PRINT 10

CPU_state cpu = {};
uint64_t g_nr_guest_inst = 0;
static uint64_t g_timer = 0; // unit: us
static bool g_print_step = false;

void device_update();

static void trace_and_difftest(Decode *_this, vaddr_t dnpc)
{
#ifdef CONFIG_ITRACE_COND
  if (ITRACE_COND)
  {
    log_write("%s\n", _this->logbuf);
  } // 将这些内容写入日志

#endif
  if (g_print_step)
  {
    IFDEF(CONFIG_ITRACE, puts(_this->logbuf));
  }                                                       // 根据g_print_step与CONFIG_ITRACE，确定是否打印_this->logbuf
  IFDEF(CONFIG_DIFFTEST, difftest_step(_this->pc, dnpc)); // static inline void difftest_step(vaddr_t pc, vaddr_t npc) {}
  //监视点的添加
  int num = check_watchpoint_all();
  if (num != -1)
  {
    nemu_state.state = NEMU_STOP;
    Log("No.%d watchpoint has been trigger", num);
  }
}

/*typedef struct Decode {
  vaddr_t pc;
  vaddr_t snpc; // static next pc
  vaddr_t dnpc; // dynamic next pc
  ISADecodeInfo isa;
  IFDEF(CONFIG_ITRACE, char logbuf[128]);*/
static void exec_once(Decode *s, vaddr_t pc)
{
  s->pc = pc;
  s->snpc = pc;     // 跟踪下一条要执行的指令
  isa_exec_once(s); // isa执行一次,这个真的有点困难，需要认真再看看
  cpu.pc = s->dnpc; // dnpc 是执行完当前指令后准备执行的下一条指令地址。

#ifdef CONFIG_ITRACE // 写入日志
  char *p = s->logbuf;
  p += snprintf(p, sizeof(s->logbuf), FMT_WORD ":", s->pc);
  // printf("SWT_WORD:%s\ns->ps:%x\n",FMT_WORD":",s->pc);
  // int snprintf(char *str, size_t size, const char *format, ...);
  // 用于格式化输出字符串，并将结果写入到指定的缓冲区，与 sprintf() 不同的是，snprintf() 会限制输出的字符数，避免缓冲区溢出。
  int ilen = s->snpc - s->pc; // 下一指令地址和当前指令地址的差值(指令长度)
  int i;
  uint8_t *inst = (uint8_t *)&s->isa.inst.val;
  //  printf("s->isa.inst.val: 0x%X\n", s->isa.inst.val);

  for (i = ilen - 1; i >= 0; i--)
  {
    p += snprintf(p, 4, " %02x", inst[i]); // 将指令的字节格式化为十六进制字符串并写入日志缓冲区。
    // printf("inst[%d]:%X\n",i,inst[i]);
  }
  int ilen_max = MUXDEF(CONFIG_ISA_x86, 8, 4);
  // printf("ilen_max:%d\n",ilen_max);
  int space_len = ilen_max - ilen;
  // printf("space_len:%d\n",space_len);
  if (space_len < 0)
    space_len = 0;
  space_len = space_len * 3 + 1;
  memset(p, ' ', space_len); // 用空格填充日志缓冲区中的空白区域。
  p += space_len;

#ifndef CONFIG_ISA_loongarch32r
  void disassemble(char *str, int size, uint64_t pc, uint8_t *code, int nbyte); // 调用反汇编函数，将指令的机器码翻译为汇编语言，并写入日志。
  disassemble(p, s->logbuf + sizeof(s->logbuf) - p,
              MUXDEF(CONFIG_ISA_x86, s->snpc, s->pc), (uint8_t *)&s->isa.inst.val, ilen);
  printf("s->snpc:%x\ns->pc:%x\n",s->snpc,s->pc);
  // printf("MUXDEF(CONFIG_ISA_x86, s->snpc, s->pc):%x\n",MUXDEF(CONFIG_ISA_x86, s->snpc, s->pc));
  // printf("&s->isa.inst.val:0x%X\nilen:%x\n",s->isa.inst.val,ilen);
#else
  p[0] = '\0'; // the upstream llvm does not support loongarch32r
#endif
#endif
}

/*代码将在一个for循环中调用exec_once
  EMU将不断执行指令, 直到遇到以下情况之一, 才会退出指令执行的循环:
     1.达到要求的循环次数.
     2.客户程序执行了nemu_trap指令. 这是一条虚构的特殊指令, 它是为了在NEMU中让客户程序指示执行的结束而加入的,
     NEMU在ISA手册中选择了一些用于调试的指令, 并将nemu_trap的特殊含义赋予它们.
     例如在riscv32的手册中, NEMU选择了ebreak指令来充当nemu_trap. 为了表示客户程序是否成功结束,
     nemu_trap指令还会接收一个表示结束状态的参数. 当客户程序执行了这条指令之后, NEMU将会根据这个结束状态参数来设置NEMU的结束状态,
     并根据不同的状态输出不同的结束信息, 主要包括
       HIT GOOD TRAP - 客户程序正确地结束执行
       HIT BAD TRAP - 客户程序错误地结束执行
       ABORT - 客户程序意外终止, 并未结束执行*/
/*typedef struct Decode {
  vaddr_t pc;
  vaddr_t snpc; // static next pc
  vaddr_t dnpc; // dynamic next pc
  ISADecodeInfo isa;
  IFDEF(CONFIG_ITRACE, char logbuf[128]);
} Decode;*/
static void execute(uint64_t n)
{
  Decode s;
  int i = 0;
  for (; i < n;)
  {
    i++;
    exec_once(&s, cpu.pc);

    // 监控点控制
    g_nr_guest_inst++; // 记录指令数
    trace_and_difftest(&s, cpu.pc);
    if (nemu_state.state != NEMU_RUNNING)

      break;                               // 是否是继续执行的状态
    IFDEF(CONFIG_DEVICE, device_update()); // 这条需要关注
  }
  Log("execute %d times", i);
}

static void statistic()
{
  IFNDEF(CONFIG_TARGET_AM, setlocale(LC_NUMERIC, "")); // 这里还有问题
#define NUMBERIC_FMT MUXDEF(CONFIG_TARGET_AM, "%", "%'") PRIu64
  Log("host time spent = " NUMBERIC_FMT " us", g_timer);
  Log("total guest instructions = " NUMBERIC_FMT, g_nr_guest_inst);
  if (g_timer > 0)
    Log("simulation frequency = " NUMBERIC_FMT " inst/s", g_nr_guest_inst * 1000000 / g_timer);
  else
    Log("Finish running in less than 1 us and can not calculate the simulation frequency");
}

void assert_fail_msg()
{
  isa_reg_display();
  statistic();
}

/* Simulate how the CPU works. */
void cpu_exec(uint64_t n)
{
  g_print_step = (n < MAX_INST_TO_PRINT);
  switch (nemu_state.state)
  {
  case NEMU_END:
  case NEMU_ABORT:
    printf("Program execution has ended. To restart the program, exit NEMU and run again.\n");
    return;
  default:
    nemu_state.state = NEMU_RUNNING;
  }

  uint64_t timer_start = get_time(); // CPU开始执行的时间

  execute(n); // 模拟了CPU的工作方式：不断执行指令

  uint64_t timer_end = get_time();    // CPU停止执行的时间
  g_timer += timer_end - timer_start; // CPU执行的总时间

  switch (nemu_state.state)
  {
  case NEMU_RUNNING:
    nemu_state.state = NEMU_STOP;
    break; // 将运行状态置为停止状态

  case NEMU_END:
  case NEMU_ABORT:
    Log("nemu: %s at pc = " FMT_WORD,
        (nemu_state.state == NEMU_ABORT ? ANSI_FMT("ABORT", ANSI_FG_RED) : (nemu_state.halt_ret == 0 ? ANSI_FMT("HIT GOOD TRAP", ANSI_FG_GREEN) : ANSI_FMT("HIT BAD TRAP", ANSI_FG_RED))),
        nemu_state.halt_pc); // 这里还有问题
    // fall through
  case NEMU_QUIT:
    statistic(); // 停止
    //    NEMU会在cpu_exec()函数的最后打印执行的指令数目和花费的时间, 并计算出指令执行的频率.
    // 但由于内置客户程序太小, 执行很快就结束了, 目前无法计算出有意义的频率,
    // 将来运行一些复杂的程序时, 此处输出的频率可以用于粗略地衡量NEMU的性能.
    // statistic()主要做这个事儿
  }
}
