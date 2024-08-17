
// #include <memory>
// #include <verilated.h>
// #include "Vex01Seletor.h"
// #include "verilated_vcd_c.h"
// #include <nvboard.h>
// #include <cstdlib>  // 包含 rand() 函数
// #include <cstdio>   // 包含 printf 函数
// #include <cassert>  // 包含 assert 函数

// // 模拟时间戳函数vluint64_t sim_time = 0; 
// int sim_time = 10; 
// void nvboard_bind_all_pins(ex01Seletor_NAME* ex01Seletor);
// // void nvboard_bind_all_pins(Vex01Seletor* ex01Seletor) ;


// int main(int argc, char **argv) {
//     // 防止未使用的参数警告
//     if (false && argc && argv) {}
    
    
//     // 创建 Verilator 上下文
//     const std::unique_ptr<VerilatedContext> contextp{new VerilatedContext};
//     contextp->debug(0);
//     contextp->randReset(2);  // 随机初始化信号
//     contextp->traceEverOn(true);  // 启用波形追踪
//     contextp->commandArgs(argc, argv);

//     // 创建顶层模块实例
//     const std::unique_ptr<Vex01Seletor> ex01Seletor{new Vex01Seletor{contextp.get(), "ex01Seletor"}};

//     // 初始化 VCD 波形生成
//     VerilatedVcdC *tfp = new VerilatedVcdC;
//     ex01Seletor->trace(tfp, 0);
//     tfp->open("wave.vcd");  // 设置波形输出文件为 wave.vcd

//     nvboard_bind_all_pins(ex01Seletor.get());
//     nvboard_init();
    

//     // // 仿真主循环
//     // while (!contextp->gotFinish() && contextp->time() < sim_time){  
//     //     // 随机生成 a 和 b 的值
//     //     int a = rand() & 1;
//     //     int b = rand() & 1;
//     //     ex01Seletor->a = a;
//     //     ex01Seletor->b = b;

//     //     // 评估模块
//     //     ex01Seletor->eval();

//     //     // 记录当前时间的波形
//     //     tfp->dump(contextp->time());

//     //     // 打印当前状态
//     //     printf("a = %d, b = %d, f = %d\n", a, b, ex01Seletor->f);

//     //     // 断言检查逻辑
//     //     assert(ex01Seletor->f == (a ^ b));

//     //     // 增加时间
//     //     contextp->timeInc(1);

//     //     nvboard_update();
//     // }


//     while(1)
//     {
        
//         ex01Seletor->eval();
        
//         nvboard_update();
//     }
    

//     // 仿真结束处理
//     nvboard_quit();
//     ex01Seletor->final();
//     tfp->close();
//     delete tfp;
//     return 0;
// }



// #include <memory>
// #include <verilated.h>
// #include "Vex01Seletor.h"
// #include "verilated_vcd_c.h"
// #include <nvboard.h>
// #include <cstdlib> // 包含 rand() 函数
// #include <cstdio>  // 包含 printf 函数
// #include <cassert> // 包含 assert 函数

// // 模拟时间戳函数vluint64_t sim_time = 0;
// int sim_time = 10;
// void nvboard_bind_all_pins(Vex01Seletor* top);
// // void nvboard_bind_all_pins(Vex01Seletor* ex01Seletor) ;

// int main(int argc, char **argv)
// {
//     // 防止未使用的参数警告
//     if (false && argc && argv)
//     {
//     }

//     // 创建 Verilator 上下文
//     const std::unique_ptr<VerilatedContext> contextp{new VerilatedContext};
//     contextp->debug(0);
//     contextp->randReset(2);      // 随机初始化信号
//     contextp->traceEverOn(true); // 启用波形追踪
//     contextp->commandArgs(argc, argv);

//     // 创建顶层模块实例
//     const std::unique_ptr<Vex01Seletor> ex01Seletor{new Vex01Seletor{contextp.get(), "ex01Seletor"}};

//     // 初始化 VCD 波形生成
//     VerilatedVcdC *tfp = new VerilatedVcdC;
//     ex01Seletor->trace(tfp, 0);
//     tfp->open("wave.vcd"); // 设置波形输出文件为 wave.vcd

//     nvboard_bind_all_pins(ex01Seletor.get());
//     nvboard_init();
//     // 仿真主循环
//     while (!contextp->gotFinish() && contextp->time() < sim_time)
//     {
//         // 随机生成 a 和 b 的值
//         int Y = std::rand() % 4;
//         int X0 = std::rand() % 4;
//         int X1 = std::rand() % 4;
//         int X2 = std::rand() % 4;
//         int X3 = std::rand() % 4;
//         ex01Seletor->Y = Y;
//         ex01Seletor->X0 = X0;
//         ex01Seletor->X1 = X1;
//         ex01Seletor->X2 = X2;
//         ex01Seletor->X3 = X3;

//         int temp;
//         switch (Y)
//         {
//         case 0:
//             temp = X0;
//             break;
//         case 1:
//             temp = X1;
//             break;
//         case 2:
//             temp = X2;
//             break;
//         case 3:
//             temp = X3;
//             break;
//         default:
//             // 处理无效值的情况
//             temp = 0; // 或者其他合理的默认值
//             break;
//         }
//         // 评估模块
//         ex01Seletor->eval();

//         // 记录当前时间的波形
//         tfp->dump(contextp->time());

//         // 打印当前状态
//         printf("Y = %d, x = %d, F = %d\n", Y, temp, ex01Seletor->F);

//         // 断言检查逻辑
//         assert(ex01Seletor->F == temp);

//         // 增加时间
//         contextp->timeInc(1);

//         nvboard_update();
//     }
//     while (1)
//     {
//         ex01Seletor->eval();
//         nvboard_update();
//     }

//     // 仿真结束处理
//     nvboard_quit();
//     ex01Seletor->final();
//     tfp->close();
//     delete tfp;
//     return 0;
// }

#include <nvboard.h>
#include <Vex01Seletor.h>

static TOP_NAME dut;

void nvboard_bind_all_pins(TOP_NAME* top);


int main() {
  nvboard_bind_all_pins(&dut);
  nvboard_init();

 

  while(1) {
    nvboard_update();
    dut.eval();
  }
}