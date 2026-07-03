NPC_VSRCS += $(abspath ./src/isa/riscv32e/packages/ALUPkg.sv)
NPC_VSRCS += $(abspath ./src/isa/riscv32e/packages/NextPCPkg.sv)
NPC_VSRCS += $(abspath ./src/isa/riscv32e/packages/RV32EPkg.sv)
NPC_VSRCS += $(shell find $(abspath ./src/isa/riscv32e/vsrc) \( -name "*.sv" -o -name "*.v" \) | sort)
