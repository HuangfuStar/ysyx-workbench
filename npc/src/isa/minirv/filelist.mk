NPC_VSRCS += $(abspath ./src/isa/minirv/MiniRVPkg.sv)
NPC_VSRCS += $(shell find $(abspath ./src/isa/minirv) \( -name "*.sv" -o -name "*.v" \) ! -name "MiniRVPkg.sv" | sort)
