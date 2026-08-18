#pragma once

enum class ALU_OPERATION {
    ADD,
    AND,
    NOT,
    PASS
};

enum class ALU_A_INPUT {
    SRC1,
    PC_PLUS_ONE
};

enum class ALU_B_INPUT {
    SRC2,
    IMM
};

enum class IMMEDIATE_FORMAT {
    NONE,
    IMM5,
    OFFSET6,
    OFFSET9,
    OFFSET11
};

enum class PC_OP {
    SEQ,
    BRANCH,
    JMP,
    SUB
};


enum class WB_OP {
    ALU,
    MEM,
    PC
};

//bool REG_WRITE;

//bool MEM_READ;

//bool MEM_WRITE;

//bool CC_WRITE;

struct controls {
    ALU_OPERATION alu_op;
    ALU_A_INPUT alu_a_input;
    ALU_B_INPUT alu_b_input;
    IMMEDIATE_FORMAT immediate_format;
    PC_OP pc_op;
    WB_OP wb_op;
    bool REG_WRITE;
    bool MEM_READ;
    bool MEM_WRITE;
    bool CC_WRITE;
};
