#include "lc3.h"

// memory
short memory[MEMORY_MAX]; 
// short because lc3 has 16 bits
// 2^16 mem locations - array will be 2^16

// registers
enum {
    R0,
    R1,
    R2,
    R3,
    R4,
    R5,
    R6,
    R7,
    R_PC, 
    R_COND
};

short reg[R_COUNT];

// instruction set 
enum {
    OP_BR,
    OP_ADD,
    OP_LD,
    OP_ST,
    OP_LDR,
    OP_STR,
    OP_LDI,
    OP_STI,
    OP_JMP,
    OP_JSR,
    OP_AND,
    OP_NOT,
    OP_RES,
    OP_RTI,
    OP_LEA,
    OP_TRAP
};

// condition flags
enum {
    FL_POS = 1 << 0,
    FL_ZER = 1 << 1,
    FL_NEG = 1 << 2,
};


int mem_read(int input) {
    return 0;
}

int sign_extend(int input, int bit_count) {
    int last_bit = input >> (bit_count - 1) & 0x1;
    int bit_mask = 0xFFFF << bit_count;
    int sext_value;
    if (last_bit) {
        return input | (bit_mask);
    }
    return input;
}

void update_flags(int output) {
    if (output > 0) {
        reg[R_COND] = FL_POS;
    } else if (output == 0) {
        reg[R_COND] = FL_ZER;
    } else {
        reg[R_COND] = FL_NEG;
    }
}

int add(int instr) {
    // if bit 5 = 0: DR = SR1 + SR2
    // if bit 5 = 1: DR = SR1 + SEXT(IMM5)

    int bit5 = (instr >> 5) & 0x1;
    int DR_REG = (instr >> 9) & 0x7;
    int SR1_REG = (instr >> 6) & 0x7;

    if (bit5) {
        int imm5 = instr & 0x1F;
        reg[DR_REG] = reg[SR1_REG] + sign_extend(imm5, 5);
    } else {
        int SR2_REG = instr & 0x7;
        reg[DR_REG] = reg[SR1_REG] + reg[SR2_REG];
    }
    update_flags(reg[DR_REG]);
}

int main(int argc, const char* argv[]) {

    reg[R_COND] = FL_ZER;

    reg[R_PC] = PC_START;

    int running = 1;
    while (running) {

        int instr = mem_read(reg[R_PC]++);
        int op = instr >> 12; // right shift by 12 bits

        switch (op) {
            case OP_ADD:
                add(instr);
                break;
            case OP_AND:
                
        }
    }


}
