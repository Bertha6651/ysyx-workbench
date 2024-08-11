
TOP = top

# Verilator 相关变量
VERILATOR = verilator
VERILATOR_FLAGS = --cc --trace --exe --build -j 4

# C++ 仿真文件
CPP_FILE = main.cpp

# 生成的仿真可执行文件名
EXE = Vtop

# 默认目标，编译和仿真
all: $(EXE)

# 编译 Verilog 并生成仿真可执行文件
$(EXE): ./vsrc/$(TOP).v ./csrc/$(CPP_FILE)
	$(VERILATOR) $(VERILATOR_FLAGS) -o Vtop ./vsrc/$(TOP).v ./csrc/$(CPP_FILE)

# 运行仿真
run: $(EXE)
	obj_dir/$(EXE)

# 查看波形
wave: $(EXE)
	gtkwave wave.vcd &

# 清理生成的文件
clean:
	rm -rf obj_dir/ wave.vcd

# # 保留 git 追踪命令
# sim: $(EXE)
# 	$(call git_commit, "sim RTL")

sim:
	$(call git_commit, "sim RTL") # DO NOT REMOVE THIS LINE!!!
	@echo "Write this Makefile by your self."

include ../Makefile
