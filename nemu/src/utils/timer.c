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

#include <common.h>
#include MUXDEF(CONFIG_TIMER_GETTIMEOFDAY, <sys/time.h>, <time.h>)

IFDEF(CONFIG_TIMER_CLOCK_GETTIME,
    static_assert(CLOCKS_PER_SEC == 1000000, "CLOCKS_PER_SEC != 1000000"));
IFDEF(CONFIG_TIMER_CLOCK_GETTIME,
    static_assert(sizeof(clock_t) == 8, "sizeof(clock_t) != 8"));

static uint64_t boot_time = 0;

//static：该关键字表示函数具有内部链接，这意味着它只能在同一个翻译单元（源文件）中被调用。
static uint64_t get_time_internal() //获取当前时间（以微秒为单位）
{
#if defined(CONFIG_TARGET_AM)//IF Application on Abstract-Machine
  uint64_t us = io_read(AM_TIMER_UPTIME).us;
#elif defined(CONFIG_TIMER_GETTIMEOFDAY)
  struct timeval now;
  gettimeofday(&now, NULL);
  uint64_t us = now.tv_sec * 1000000 + now.tv_usec;
#else
  struct timespec now;
  clock_gettime(CLOCK_MONOTONIC_COARSE, &now);//The functions clock_gettime() retrieve  the time of the specified clock clockid.
  //CLOCK_MONOTONIC_COARSE： 系统运行时间，从系统启动时开始计时，速度更快精度更低，系统休眠时不再计时（NTP与硬件时钟有问题时会影响其频率，没有验证过）。
  uint64_t us = now.tv_sec * 1000000 + now.tv_nsec / 1000;//将秒转换为微秒，将纳秒转换为微秒
#endif
  return us;
}

uint64_t get_time() {
  if (boot_time == 0) boot_time = get_time_internal();
  uint64_t now = get_time_internal();
  return now - boot_time;
}

void init_rand() {
  srand(get_time_internal());//由函数 rand 使用的随机数发生器。；；参数：seed -- 这是一个整型值，用于伪随机数生成算法播种。
  //  The srand() function sets its argument as the seed for  a  new  sequence  of
  //  pseudo-random  integers  to  be returned by rand().  These sequences are re‐
  //  peatable by calling srand() with the same seed value.


}
