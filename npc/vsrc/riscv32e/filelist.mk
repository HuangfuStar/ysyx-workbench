NPC_VSRCS += $(NPC_HOME)/vsrc/riscv32e/packages/ALUPkg.sv
NPC_VSRCS += $(NPC_HOME)/vsrc/riscv32e/packages/NextPCPkg.sv
NPC_VSRCS += $(NPC_HOME)/vsrc/riscv32e/packages/RV32EPkg.sv
NPC_VSRCS += $(shell find $(NPC_HOME)/vsrc/riscv32e \( -name "*.sv" -o -name "*.v" \) ! -path "$(NPC_HOME)/vsrc/riscv32e/packages/*" | sort)
