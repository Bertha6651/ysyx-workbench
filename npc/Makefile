
# TOP = top

# # Verilator 相关变量
# VERILATOR = verilator
# VERILATOR_FLAGS = --cc --trace --exe --build -j 4

# # C++ 仿真文件
# CPP_FILE = main.cpp

# # 生成的仿真可执行文件名
# EXE = Vtop

# # 默认目标，编译和仿真
# all: $(EXE)

# # 编译 Verilog 并生成仿真可执行文件
# $(EXE): ./vsrc/$(TOP).v ./csrc/$(CPP_FILE)
# 	$(VERILATOR) $(VERILATOR_FLAGS) -o Vtop ./vsrc/$(TOP).v ./csrc/$(CPP_FILE)

# # 运行仿真
# run: $(EXE)
# 	obj_dir/$(EXE)
TOPNAME = ex01Seletor
NXDC_FILES = constr/$(TOPNAME).nxdc
INC_PATH ?=

VERILATOR = verilator
VERILATOR_CFLAGS += -MMD --build -cc  --trace\
				-O3 --x-assign fast --x-initial fast --noassert

BUILD_DIR = ./build
OBJ_DIR = $(BUILD_DIR)/obj_dir
BIN = $(BUILD_DIR)/$(TOPNAME)

default: $(BIN)

$(shell mkdir -p $(BUILD_DIR))

# constraint file
SRC_AUTO_BIND = $(abspath $(BUILD_DIR)/auto_bind.cpp)
$(SRC_AUTO_BIND): $(NXDC_FILES)
	python3 $(NVBOARD_HOME)/scripts/auto_pin_bind.py $^ $@

# project source
VSRCS = $(shell find $(abspath ./vsrc) -name "*.v")
CSRCS = $(shell find $(abspath ./csrc) -name "*.c" -or -name "*.cc" -or -name "*.cpp")
CSRCS += $(SRC_AUTO_BIND)

# rules for NVBoard
include $(NVBOARD_HOME)/scripts/nvboard.mk

# rules for verilator
INCFLAGS = $(addprefix -I, $(INC_PATH))
CXXFLAGS += $(INCFLAGS) -DTOP_NAME="\"V$(TOPNAME)\""

$(BIN): $(VSRCS) $(CSRCS) $(NVBOARD_ARCHIVE)
	@rm -rf $(OBJ_DIR)
	$(VERILATOR) $(VERILATOR_CFLAGS) \
		--top-module $(TOPNAME) $^ \
		$(addprefix -CFLAGS , $(CXXFLAGS)) $(addprefix -LDFLAGS , $(LDFLAGS)) \
		--Mdir $(OBJ_DIR) --exe -o $(abspath $(BIN))

all: default

run: $(BIN)
	@$^

clean:
	rm -rf $(BUILD_DIR) wave.vcd


# 查看波形
wave: $(EXE)
	gtkwave wave.vcd &

# # 清理生成的文件
# clean:
# 	rm -rf obj_dir/ wave.vcd

# # # 保留 git 追踪命令
# # sim: $(EXE)
# # 	$(call git_commit, "sim RTL")

sim:
	$(call git_commit, "sim RTL") # DO NOT REMOVE THIS LINE!!!
	@echo "Write this Makefile by your self."

include  ../Makefile

.PHONY: default all clean run wave sim
