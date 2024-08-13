
#include <memory>
#include <verilated.h>
#include "Vtop.h"
#include "verilated_vcd_c.h"
#include <nvboard.h>
#include <cstdlib>  // 包含 rand() 函数
#include <cstdio>   // 包含 printf 函数
#include <cassert>  // 包含 assert 函数

// 模拟时间戳函数vluint64_t sim_time = 0; 
int sim_time = 999; 
void nvboard_bind_all_pins(TOP_NAME* top);
// void nvboard_bind_all_pins(Vtop* top) ;


int main(int argc, char **argv) {
    // 防止未使用的参数警告
    if (false && argc && argv) {}
    
    
    // 创建 Verilator 上下文
    const std::unique_ptr<VerilatedContext> contextp{new VerilatedContext};
    contextp->debug(0);
    contextp->randReset(2);  // 随机初始化信号
    contextp->traceEverOn(true);  // 启用波形追踪
    contextp->commandArgs(argc, argv);

    // 创建顶层模块实例
    const std::unique_ptr<Vtop> top{new Vtop{contextp.get(), "TOP"}};

    // 初始化 VCD 波形生成
    VerilatedVcdC *tfp = new VerilatedVcdC;
    top->trace(tfp, 0);
    tfp->open("wave.vcd");  // 设置波形输出文件为 wave.vcd

    nvboard_bind_all_pins(top.get());
    nvboard_init();
    
    while(1)
    {
        top->eval();
        nvboard_update();
    }
    // 仿真主循环
    while (!contextp->gotFinish() && contextp->time() < sim_time){  
        // 随机生成 a 和 b 的值
        int a = rand() & 1;
        int b = rand() & 1;
        top->a = a;
        top->b = b;

        // 评估模块
        top->eval();

        // 记录当前时间的波形
        tfp->dump(contextp->time());

        // 打印当前状态
        printf("a = %d, b = %d, f = %d\n", a, b, top->f);

        // 断言检查逻辑
        assert(top->f == (a ^ b));

        // 增加时间
        contextp->timeInc(1);

        nvboard_update();
    }

    // 仿真结束处理
    nvboard_quit();
    top->final();
    tfp->close();
    delete tfp;
    return 0;
}
