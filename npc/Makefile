TOPNAME = Top
ISA ?= riscv32e
VERILATOR = verilator
VALID_ISAS := minirv riscv32e

NPC_HOME ?= $(CURDIR)
ifeq ($(wildcard $(NPC_HOME)/csrc/npc-main.cpp),)
  $(error NPC_HOME=$(NPC_HOME) is not an npc repo)
endif

# trace function switch
CONFIG_ITRACE ?= 0
CONFIG_MTRACE ?= 0
CONFIG_FTRACE ?= 1
CONFIG_IRINGBUF ?= 1
CONFIG_IRINGBUF_NR ?= 16
CONFIG_DIFFTEST ?= 0
DIFF ?= $(NEMU_HOME)/build/riscv32-nemu-interpreter-so

# basic target setup
WORK_DIR = $(shell pwd)
BUILD_DIR = $(WORK_DIR)/build
OBJ_DIR = $(BUILD_DIR)/obj_dir
BIN = $(BUILD_DIR)/$(TOPNAME)
WAVE = $(BUILD_DIR)/wave.fst
IMG ?=
ELF ?= $(subst .bin,.elf,$(IMG))

# check the validility of the ISA
ifneq ($(MAKECMDGOALS),clean)
ifeq ($(filter $(ISA),$(VALID_ISAS)),)
$(error Unsupported ISA '$(ISA)'. Supported ISA values: $(VALID_ISAS))
endif
endif

# source code inclusion
NPC_VSRCS :=
NPC_CSRCS :=
NPC_INC := -I$(NPC_HOME)/include
include $(NPC_HOME)/csrc/filelist.mk
ifneq ($(MAKECMDGOALS),clean)
include $(NPC_HOME)/vsrc/filelist.mk
endif
VSRCS = $(NPC_VSRCS)
CSRCS = $(NPC_CSRCS)

# verilator flags setup
# TODO: using menuconfig to substitute the hard encoding
VERILATOR_FLAGS += -Wall --cc --exe --build --trace-fst
VERILATOR_FLAGS += -O3 --x-assign fast --x-initial fast --noassert
VERILATOR_FLAGS += $(if $(filter riscv32e,$(ISA)),-DCONFIG_ISA_RISCV32E,-DCONFIG_ISA_MINIRV)
VERILATOR_FLAGS += -DCONFIG_DEBUG
ifneq ($(CONFIG_ITRACE),0)
VERILATOR_FLAGS += -DCONFIG_ITRACE
endif
ifneq ($(CONFIG_MTRACE),0)
VERILATOR_FLAGS += -DCONFIG_MTRACE
endif
ifneq ($(CONFIG_FTRACE),0)
VERILATOR_FLAGS += -DCONFIG_FTRACE
endif
ifneq ($(CONFIG_IRINGBUF),0)
VERILATOR_FLAGS += -DCONFIG_IRINGBUF -DCONFIG_IRINGBUF_NR=$(CONFIG_IRINGBUF_NR)
endif
ifneq ($(CONFIG_DIFFTEST),0)
VERILATOR_FLAGS += -DCONFIG_DIFFTEST
endif

TRACE_CFLAGS = -DCONFIG_DEBUG --std=c++20
ifneq ($(CONFIG_ITRACE),0)
TRACE_CFLAGS += -DCONFIG_ITRACE
endif
ifneq ($(CONFIG_MTRACE),0)
TRACE_CFLAGS += -DCONFIG_MTRACE
endif
ifneq ($(CONFIG_FTRACE),0)
TRACE_CFLAGS += -DCONFIG_FTRACE
endif
ifneq ($(CONFIG_IRINGBUF),0)
TRACE_CFLAGS += -DCONFIG_IRINGBUF -DCONFIG_IRINGBUF_NR=$(CONFIG_IRINGBUF_NR)
endif
ifneq ($(CONFIG_DIFFTEST),0)
TRACE_CFLAGS += -DCONFIG_DIFFTEST
endif

# link the capstone
TRACE_LDFLAGS =
LIBCAPSTONE =
ifneq ($(CONFIG_ITRACE)$(CONFIG_IRINGBUF),00)
LIBCAPSTONE = $(abspath ../nemu/tools/capstone/repo/libcapstone.so.5)
TRACE_CFLAGS += -I$(abspath ../nemu/tools/capstone/repo/include)
TRACE_LDFLAGS += -ldl # for dlopen/dlsym/dlclose
endif
ifneq ($(CONFIG_DIFFTEST),0)
TRACE_LDFLAGS += -ldl
endif

VERILATOR_FLAGS += --CFLAGS "$(TRACE_CFLAGS) $(NPC_INC)"
ifneq ($(strip $(TRACE_LDFLAGS)),)
VERILATOR_FLAGS += --LDFLAGS "$(TRACE_LDFLAGS)"
endif

# target specification
.PHONY: default run sim wave clean

default: $(BIN)

$(shell mkdir -p $(BUILD_DIR))

$(BIN): $(VSRCS) $(CSRCS) $(LIBCAPSTONE)
	$(call git_commit, "sim RTL") # DO NOT REMOVE THIS LINE!!!
	@mkdir -p $(BUILD_DIR)
	@rm -rf $(OBJ_DIR)
	$(VERILATOR) $(VERILATOR_FLAGS) \
		--top-module $(TOPNAME) \
		$(VSRCS) $(CSRCS) \
		--Mdir $(OBJ_DIR) \
		-o $(abspath $(BIN))

run: $(BIN)
	$(BIN) -b $(if $(ELF),-e $(ELF)) $(IMG)

sim: run

wave: run
	@echo $(WAVE)
	gtkwave $(WAVE)

clean:
	rm -rf $(BUILD_DIR)

include ../Makefile

ifneq ($(strip $(LIBCAPSTONE)),)
$(LIBCAPSTONE):
	$(MAKE) -C ../nemu/tools/capstone
endif
