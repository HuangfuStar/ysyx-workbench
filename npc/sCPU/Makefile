TOPNAME = sCPU
VERILATOR = verilator

BUILD_DIR = ./build
OBJ_DIR = $(BUILD_DIR)/obj_dir
BIN = $(BUILD_DIR)/$(TOPNAME)
WAVE = $(BUILD_DIR)/wave.fst

VSRCS = $(shell find $(abspath ./vsrc) $(abspath ../common) -name "*.v")
CSRCS = $(shell find $(abspath ./csrc) -name "*.c" -o -name "*.cc" -o -name "*.cpp")

VERILATOR_FLAGS += -Wall --cc --exe --build --trace-fst
VERILATOR_FLAGS += -O3 --x-assign fast --x-initial fast --noassert

default: $(BIN)

$(shell mkdir -p $(BUILD_DIR))

$(BIN): $(VSRCS) $(CSRCS)
	@mkdir -p $(BUILD_DIR)
	$(VERILATOR) $(VERILATOR_FLAGS) \
		--top-module $(TOPNAME) \
		$(VSRCS) $(CSRCS) \
		--Mdir $(OBJ_DIR) \
		-o $(abspath $(BIN))

run: $(BIN)
	@$(BIN)

wave: $(WAVE)
	gtkwave $(WAVE)

$(WAVE): $(BIN)
	@$(BIN)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: default run wave clean
