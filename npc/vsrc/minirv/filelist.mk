NPC_VSRCS += $(NPC_HOME)/vsrc/minirv/MiniRVPkg.sv
NPC_VSRCS += $(shell find $(NPC_HOME)/vsrc/minirv \( -name "*.sv" -o -name "*.v" \) ! -name "MiniRVPkg.sv" | sort)
