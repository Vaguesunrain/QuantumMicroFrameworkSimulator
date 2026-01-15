MODULE =module_top

OBJ_DIR = obj_dir
# --- 修改点：使用绝对路径 ---
INC_DIR = $(abspath cppFiles/head)
SRC_DIR = cppFiles/src
SV_DIR = svFiles
EIGEN3_INC_DIR = /usr/include/eigen3

# 源文件列表
TB_CPPS = sim_main.cpp $(shell find $(SRC_DIR) -name "*.cpp")# traverse all cpp files in src
SV_FILES = -f files.f

# --- 修改点：这里会自动展开为绝对路径 ---
CFLAGS = -I$(INC_DIR) -I$(EIGEN3_INC_DIR) 
V_FLAGS = -Wall --trace --cc --exe
V_FLAGS += -CFLAGS "$(CFLAGS)"

EXE = $(OBJ_DIR)/V$(MODULE)

.PHONY: all build run wave clean

all: run

SV_SOURCES = $(shell find $(SV_DIR) -name "*.sv" -o -name "*.v")

$(OBJ_DIR)/V$(MODULE).mk: files.f $(SV_SOURCES)
	@echo "--- [Verilator] Generating C++ files (SV source changed) ---"
	verilator $(V_FLAGS) $(SV_FILES) $(TB_CPPS) --Mdir $(OBJ_DIR)

build: $(OBJ_DIR)/V$(MODULE).mk
	@echo "--- [Make] Compiling C++ ---"
	$(MAKE) -j -C $(OBJ_DIR) -f V$(MODULE).mk


run: build
	@echo "--- [Sim] Running Simulation ---"
	./$(EXE)

wave:
	gtkwave wave.vcd &

clean:
	rm -rf $(OBJ_DIR)
	rm -f *.vcd *.log