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

void init_rand();
void init_log(const char *log_file);
void init_mem();
void init_difftest(char *ref_so_file, long img_size, int port);
void init_device();
void init_sdb();
void init_disasm(const char *triple);

static void welcome() //输出了各种信息
{
  Log("Trace: %s", MUXDEF(CONFIG_TRACE, ANSI_FMT("ON", ANSI_FG_GREEN), ANSI_FMT("OFF", ANSI_FG_RED)));
  IFDEF(CONFIG_TRACE, Log("If trace is enabled, a log file will be generated "
        "to record the trace. This may lead to a large log file. "
        "If it is not necessary, you can disable it in menuconfig"));
  Log("Build time: %s, %s", __TIME__, __DATE__);
  printf("Welcome to %s-NEMU!\n", ANSI_FMT(str(__GUEST_ISA__), ANSI_FG_YELLOW ANSI_BG_RED));
  printf("For help, type \"help\"\n");
  assert(1);//8-25 日修改了一版
}
/*概念：C/C++ 中的 assert 是一个宏，用于在运行时检查一个条件是否为真，如果条件不满足，则运行时将终止程序的执行并输出一条错误信息。
如果这个语句的结果为 false，assert 宏就会以"Assertion failed: , file , line "的形式显示出错信息，然后使程序崩溃并终止运行。如果该语句的结果为 true，则 assert 宏不做任何操作。
*/
#ifndef CONFIG_TARGET_AM
#include <getopt.h>

void sdb_set_batch_mode();

static char *log_file = NULL;
static char *diff_so_file = NULL;
static char *img_file = NULL;
static int difftest_port = 1234;

static long load_img()//这个函数会将一个有意义的客户程序从镜像文件读入到内存, 覆盖刚才的内置客户程序. 
 {
  if (img_file == NULL) {
    Log("No image is given. Use the default build-in image.");
    return 4096; // built-in image size
  }

  FILE *fp = fopen(img_file, "rb");
  Assert(fp, "Can not open '%s'", img_file);
//  assert - abort the program if assertion is false
  fseek(fp, 0, SEEK_END);//#define SEEK_END	2	/* Seek from end of file.  */
  /*C 库函数 int fseek(FILE *stream, long int offset, int whence) 
  设置流 stream 的文件位置为给定的偏移 offset，参数 offset 意味着从给定的 whence 位置查找的字节数。
        常量	  描述
    SEEK_SET	文件的开头
    SEEK_CUR	文件指针的当前位置
    SEEK_END	文件的末尾  
*/
  long size = ftell(fp);//把指针移动到末尾才能等到文件的size
//The ftell() function obtains the current value of the file position indicator for the stream pointed to by stream.

  Log("The image is %s, size = %ld", img_file, size);
  printf("The image is %s, size = %ld", img_file, size);

  fseek(fp, 0, SEEK_SET);//#define SEEK_SET	0	/* Seek from beginning of file.  */移动到搜捕
  int ret = fread(guest_to_host(RESET_VECTOR), size, 1, fp);//对RESET_VECTOR理解不够//这里应该是核心部分
  assert(ret == 1);

  fclose(fp);
  return size;
}

/*int getopt_long(int argc, char * const argv[],
                  const char *optstring,
                  const struct option *longopts, int *longindex);*/

/*上面这个 optstring 在传入之后，getopt 函数将依次检查命令行是否指定了 -a， -b， -c(这需要多次调用 getopt 函数，直到其返回-1)，
当检查到上面某一个参数被指定时，函数会返回被指定的参数名称(即该字母)

optarg —— 指向当前选项参数(如果有)的指针。
optind —— 再次调用 getopt() 时的下一个 argv指针的索引。
optopt —— 最后一个未知选项。
opterr ­—— 如果不希望getopt()打印出错信息，则只要将全域变量opterr设为0即可。*/

static int parse_args(int argc, char *argv[]) {
  //struct option(const char *name;int has_arg;int *flag;int val;)

  const struct option table[] = {                 
    {"batch"    , no_argument      , NULL, 'b'},
    {"log"      , required_argument, NULL, 'l'},
    {"diff"     , required_argument, NULL, 'd'},
    {"port"     , required_argument, NULL, 'p'},
    {"help"     , no_argument      , NULL, 'h'},
    {0          , 0                , NULL,  0 },
  };
  int o;
  while ( (o = getopt_long(argc, argv, "-bhl:d:p:", table, NULL)) != -1) {
    switch (o) {
      case 'b': sdb_set_batch_mode(); break;//is_batch_mode = true;
      case 'p': sscanf(optarg, "%d", &difftest_port); break;//optarg:指向当前选项参数(如果有)的指针。static int difftest_port = 1234;
      case 'l': log_file = optarg; break;//static char *log_file = NULL;
      case 'd': diff_so_file = optarg; break;
      case 1: img_file = optarg; return 0;//注意这里是没有参数传入，同时没有break
      default:
        printf("Usage: %s [OPTION...] IMAGE [args]\n\n", argv[0]);
        printf("\t-b,--batch              run with batch mode\n");
        printf("\t-l,--log=FILE           output log to FILE\n");
        printf("\t-d,--diff=REF_SO        run DiffTest with reference REF_SO\n");
        printf("\t-p,--port=PORT          run DiffTest with port PORT\n");//打印信息
        printf("\n");
        exit(0);
    }
  }
  return 0;
}

void init_monitor(int argc, char *argv[]) {
  /* Perform some global initialization. */

  /* Parse arguments. */
  parse_args(argc, argv);//处理传入的参数

  /* Set random seed. */
  init_rand();

  /* Open the log file. */
  init_log(log_file);//log_file在parse_args中有体现

  /* Initialize memory. */
  init_mem();//初始化内存,这里还有问题

  /* Initialize devices. */
  IFDEF(CONFIG_DEVICE, init_device());//根据是否有没有定义CONFIG_DEVICE,进行init_device

  /* Perform ISA dependent initialization. */
  init_isa();

  /* Load the image to memory. This will overwrite the built-in image. */
  long img_size = load_img();//返回的是文件的长度

  /* Initialize differential testing. */
  init_difftest(diff_so_file, img_size, difftest_port);//这个部分还要好好琢磨

  /* Initialize the simple debugger. */
  init_sdb();

#ifndef CONFIG_ISA_loongarch32r
  IFDEF(CONFIG_ITRACE, init_disasm(
    MUXDEF(CONFIG_ISA_x86,     "i686",
    MUXDEF(CONFIG_ISA_mips32,  "mipsel",
    MUXDEF(CONFIG_ISA_riscv,
      MUXDEF(CONFIG_RV64,      "riscv64",
                               "riscv32"),
                               "bad"))) "-pc-linux-gnu"
  ));
#endif

  /* Display welcome message. */
  welcome();
}
#else // CONFIG_TARGET_AM
static long load_img() {
  extern char bin_start, bin_end;
  size_t size = &bin_end - &bin_start;
  Log("img size = %ld", size);
  memcpy(guest_to_host(RESET_VECTOR), &bin_start, size);
  return size;
}

void am_init_monitor() {
  init_rand();
  init_mem();
  init_isa();
  load_img();
  IFDEF(CONFIG_DEVICE, init_device());
  welcome();
}
#endif
