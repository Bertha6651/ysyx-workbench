// #include <verilated.h>
// #include <verilated_vcd_c.h>
// #include "Vtop.h" // 包含Verilator生成的顶层模块头文件

// int main(int argc, char** argv, char** env) {
//     // 初始化 Verilator
//     Verilated::commandArgs(argc, argv);
//     Vtop* top = new Vtop; // 创建顶层模块实例

//     // 初始化波形生成
//     VerilatedVcdC* tfp = nullptr;
//     Verilated::traceEverOn(true);
//     const char* vcdfile = "waveform.vcd";

//         tfp = new VerilatedVcdC;
//         top->trace(tfp, 99);  // 追踪深度为99
//         tfp->open(vcdfile);   // 打开波形文件
    

//     // 运行仿真循环
//     while (!Verilated::gotFinish()) {
//         // 生成随机输入
//         int a = rand() & 1;
//         int b = rand() & 1;
//         top->a = a;
//         top->b = b;

//         // 评估电路
//         top->eval();

//         // 打印输入和输出
//         printf("a = %d, b = %d, f = %d\n", a, b, top->f);

//         // 检查输出是否正确
//         assert(top->f == (a ^ b));

//         // 如果启用了波形追踪，记录当前时间的波形
//         if (tfp) {
//             tfp->dump(Verilated::time());
//         }

//         // 增加仿真时间
//         Verilated::timeInc(1);
//     }

//     // 清理
//     if (tfp) {
//         tfp->close(); // 关闭波形文件
//         delete tfp;
//     }
//     delete top;
//     return 0;
// }
#include <memory>
#include <verilated.h>
#include "Vtop.h"
#include "verilated_vcd_c.h"
#include <cstdlib>  // 包含 rand() 函数
#include <cstdio>   // 包含 printf 函数
#include <cassert>  // 包含 assert 函数

// 模拟时间戳函数
double sc_time_stamp() { return 0; }

int main(int argc, char **argv) {
    // 防止未使用的参数警告
    if (false && argc && argv) {}
    
    // nvboard_bind_all_pins(&dut);
    // nvboard_init();
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

    // 仿真主循环
    for (int i = 0; i < 100; ++i) {  // 运行 100 次仿真循环
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
    }

    // 仿真结束处理
    top->final();
    tfp->close();
    delete tfp;
    return 0;
}
