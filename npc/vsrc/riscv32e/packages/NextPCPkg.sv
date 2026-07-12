package NextPCPkg;
    typedef enum logic [2:0] {
        NEXTPC_JAL,
        NEXTPC_A4,
        NEXTPC_BRANCH,
        NEXTPC_JALR,
        NEXTPC_TRAP,
        NEXTPC_MRET
    } NextPCctr_t;
endpackage
