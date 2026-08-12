#include "lc3.h"
#include <cstdio>
#include <cstdlib>

// memory
int memory[MEMORY_MAX]; 
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

int reg[R_COUNT];

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

void mem_write(int address, int value) {
    return;
}

int sign_extend(int input, int bit_count) {
    int last_bit = input >> (bit_count - 1) & 0x1;
    if (last_bit) {
        int bit_mask = 0xFFFF << bit_count;
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

void add_fn(int instr) {
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

void not_fn(int instr) {
    int DR_REG = (instr >> 9) & 0x7;
    int SR_REG = (instr >> 6) & 0x7;

    reg[DR_REG] = reg[SR_REG] ^ 0xFFFF;
    update_flags(reg[DR_REG]);
}

void ld_fn(int instr) {
    int DR_REG = (instr >> 9) & 0x7;
    int PCOFF9 = sign_extend(instr & 0x1FF, 9);
    reg[DR_REG] = mem_read(reg[R_PC] + PCOFF9);
    update_flags(reg[DR_REG]);
}

void st_fn(int instr) {
    int SR_REG = (instr >> 9) & 0x7;
    int PCOFF9 = sign_extend(instr & 0x1FF, 9);
    mem_write(reg[R_PC] + PCOFF9, reg[SR_REG]);
}

void br_fn(int instr) {
    int n = ((instr >> 11) & 0x1) && (reg[R_COND] == FL_NEG);
    int z = (instr >> 10) & 0x1 && (reg[R_COND] == FL_ZER);
    int p = (instr >> 9) & 0x1 && (reg[R_COND] == FL_POS);
    
    if (n || z || p) {
        int PCOFF9 = sign_extend(instr & 0x1FF, 9);
        reg[R_PC] = reg[R_PC] + PCOFF9;
    }
}

void jsr_fn(int instr) {
    int PCOFF11 = instr & 0x7FF;
    reg[R7] = reg[R_PC];
    reg[R_PC] = reg[R_PC] + sign_extend(PCOFF11, 11);

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
                add_fn(instr);
                break;
            case OP_NOT:
                not_fn(instr);
                break;
            case OP_LD:
                ld_fn(instr);
                break;
            case OP_ST:
                st_fn(instr);
                break;
            case OP_BR:
                br_fn(instr);
                break;
            case OP_JSR:
                jsr_fn(instr);
                break;
        }
    }


}
