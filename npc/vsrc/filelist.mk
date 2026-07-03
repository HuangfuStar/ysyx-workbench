NPC_VSRCS += $(shell find $(NPC_HOME)/vsrc/common $(NPC_HOME)/common \( -name "*.sv" -o -name "*.v" \) | sort)
include $(NPC_HOME)/vsrc/$(ISA)/filelist.mk
