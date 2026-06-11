TOPNAME = Top
VERILATOR = verilator

BUILD_DIR = ./build
OBJ_DIR = $(BUILD_DIR)/obj_dir
BIN = $(BUILD_DIR)/$(TOPNAME)
WAVE = $(BUILD_DIR)/wave.fst
IMG ?=

VSRCS = $(abspath ./src/vsrc/MiniRVPkg.sv) \
	$(shell find $(abspath ./src/vsrc) $(abspath ./common) \( -name "*.sv" -o -name "*.v" \) ! -name "MiniRVPkg.sv" | sort)
CSRCS = $(shell find $(abspath ./src/csrc) \( -name "*.c" -o -name "*.cc" -o -name "*.cpp" \) | sort)

VERILATOR_FLAGS += -Wall --cc --exe --build --trace-fst
VERILATOR_FLAGS += -O3 --x-assign fast --x-initial fast --noassert

default: $(BIN)

$(shell mkdir -p $(BUILD_DIR))

$(BIN): $(VSRCS) $(CSRCS)
	$(call git_commit, "sim RTL") # DO NOT REMOVE THIS LINE!!!
	@mkdir -p $(BUILD_DIR)
	$(VERILATOR) $(VERILATOR_FLAGS) \
		--top-module $(TOPNAME) \
		$(VSRCS) $(CSRCS) \
		--Mdir $(OBJ_DIR) \
		-o $(abspath $(BIN))

run: $(BIN)
	-@$(BIN) $(IMG)

sim: run

wave: run
	@echo $(WAVE)
	gtkwave $(WAVE)

clean:
	rm -rf $(BUILD_DIR)

inclue ../Makefile

.PHONY: default run sim wave clean
