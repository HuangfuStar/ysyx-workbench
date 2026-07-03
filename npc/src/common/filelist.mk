NPC_CSRCS += $(shell find $(abspath $(NPC_HOME)/src/common/csrc) \( -name "*.c" -o -name "*.cc" -o -name "*.cpp" \) | sort)
NPC_VSRCS += $(shell find $(abspath $(NPC_HOME)/src/common/vsrc) $(abspath $(NPC_HOME)/common) \( -name "*.sv" -o -name "*.v" \) | sort)
