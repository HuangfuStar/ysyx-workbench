NPC_CSRCS += $(shell find $(NPC_HOME)/csrc -type f \( -iname "*.cpp" -o -iname "*.c" \) ! -path "$(NPC_HOME)/csrc/difftest/*" | sort)
