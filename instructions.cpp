#include <cstdint>
#include "utils.h"
#include "lc3.h"

void add_fn(uint16_t instr, uint16_t* reg) {
    // if bit 5 = 0: DR = SR1 + SR2
    // if bit 5 = 1: DR = SR1 + SEXT(IMM5)

    int bit5 = (instr >> 5) & 0x1;
    int DR_REG = (instr >> 9) & 0x7;
    int SR1_REG = (instr >> 6) & 0x7;

    if (bit5) {
        uint16_t imm5 = instr & 0x1F;
        reg[DR_REG] = reg[SR1_REG] + sign_extend(imm5, 5);
    } else {
        int SR2_REG = instr & 0x7;
        reg[DR_REG] = reg[SR1_REG] + reg[SR2_REG];
    }

    update_flags(reg[DR_REG], reg);
}


void not_fn(uint16_t instr, uint16_t* reg) {
    int DR_REG = (instr >> 9) & 0x7;
    int SR_REG = (instr >> 6) & 0x7;

    reg[DR_REG] = reg[SR_REG] ^ 0xFFFF;

    update_flags(reg[DR_REG], reg);
}


void ld_fn(uint16_t instr, uint16_t* reg, uint16_t* memory) {
    int DR_REG = (instr >> 9) & 0x7;
    uint16_t PCOFF9 = sign_extend(instr & 0x1FF, 9);

    reg[DR_REG] = mem_read(reg[R_PC] + PCOFF9, memory);

    update_flags(reg[DR_REG], reg);
}


void st_fn(uint16_t instr, uint16_t* reg, uint16_t* memory) {
    int SR_REG = (instr >> 9) & 0x7;
    uint16_t PCOFF9 = sign_extend(instr & 0x1FF, 9);

    mem_write(reg[R_PC] + PCOFF9, reg[SR_REG], memory);
}


void br_fn(uint16_t instr, uint16_t* reg) {
    int n = ((instr >> 11) & 0x1) && (reg[R_COND] == FL_NEG);
    int z = ((instr >> 10) & 0x1) && (reg[R_COND] == FL_ZER);
    int p = ((instr >> 9) & 0x1) && (reg[R_COND] == FL_POS);

    if (n || z || p) {
        uint16_t PCOFF9 = sign_extend(instr & 0x1FF, 9);
        reg[R_PC] = reg[R_PC] + PCOFF9;
    }
}


void jsr_fn(uint16_t instr, uint16_t* reg) {
    uint16_t PCOFF11 = instr & 0x7FF;

    reg[R7] = reg[R_PC];
    reg[R_PC] = reg[R_PC] + sign_extend(PCOFF11, 11);
}
