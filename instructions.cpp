#include <cstdint>
#include "utils.h"
#include "lc3.h"

void add_fn(uint16_t instr, uint16_t* reg) {
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

void and_fn(uint16_t instr, uint16_t* reg) {
    int bit5 = (instr >> 5) & 0x1;
    int DR_REG = (instr >> 9) & 0x7;
    int SR1_REG = (instr >> 6) & 0x7;

    if (bit5) {
        uint16_t imm5 = instr & 0x1F;
        reg[DR_REG] = reg[SR1_REG] & sign_extend(imm5, 5);
    } else {
        int SR2_REG = instr & 0x7;
        reg[DR_REG] = reg[SR1_REG] & reg[SR2_REG];
    }

    update_flags(reg[DR_REG], reg);
}


void not_fn(uint16_t instr, uint16_t* reg) {
    int DR_REG = (instr >> 9) & 0x7;
    int SR_REG = (instr >> 6) & 0x7;

    reg[DR_REG] = reg[SR_REG] ^ 0xFFFF;

    update_flags(reg[DR_REG], reg);
}

void jmp_fn(uint16_t instr, uint16_t* reg) {
    int BASE_REG = (instr >> 6) & 0x7;
    reg[R_PC] = reg[BASE_REG];
}

void ld_fn(uint16_t instr, uint16_t* reg, uint16_t* memory) {
    int DR_REG = (instr >> 9) & 0x7;
    uint16_t PCOFF9 = sign_extend(instr & 0x1FF, 9);

    reg[DR_REG] = mem_read(reg[R_PC] + PCOFF9, memory);

    update_flags(reg[DR_REG], reg);
}

void ldi_fn(uint16_t instr, uint16_t* reg, uint16_t* memory) {
    int PCOFF9 = instr & 0x1FF;
    int DR_REG = (instr >> 0x9) & 0x7;
    uint16_t intermediate = mem_read(reg[R_PC] + sign_extend(PCOFF9, 9), memory);
    reg[DR_REG] = mem_read(intermediate, memory);
    update_flags(reg[DR_REG], reg);
}

void ldr_fn(uint16_t instr, uint16_t* reg, uint16_t* memory) {
    int PCOFF6 = instr & 0x3F;
    int DR_REG = (instr >> 0x9) & 0x7;
    int BASE_REG = (instr >> 6) & 0x7;
    reg[DR_REG] = mem_read(reg[BASE_REG] + sign_extend(PCOFF6, 6), memory);
    update_flags(reg[DR_REG], reg);
}


void st_fn(uint16_t instr, uint16_t* reg, uint16_t* memory) {
    int SR_REG = (instr >> 9) & 0x7;
    uint16_t PCOFF9 = sign_extend(instr & 0x1FF, 9);
    mem_write(reg[R_PC] + PCOFF9, reg[SR_REG], memory);
}

void sti_fn(uint16_t instr, uint16_t* reg, uint16_t* memory) {
    int PCOFF9 = instr & 0x1FF;
    int SR_REG = (instr >> 0x9) & 0x7;
    int intermediate = mem_read(reg[R_PC] + sign_extend(PCOFF9, 9), memory);
    mem_write(intermediate, reg[SR_REG], memory);
}

void str_fn(uint16_t instr, uint16_t* reg, uint16_t* memory) {
    int PCOFF6 = instr & 0x3F;
    int SR_REG = (instr >> 0x9) & 0x7;
    int BASE_REG = (instr >> 6) & 0x7;
    mem_write(reg[BASE_REG] + sign_extend(PCOFF6, 6), reg[SR_REG], memory);
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

// also jsrr
void jsr_fn(uint16_t instr, uint16_t* reg) {
    int bit11 = (instr >> 11) & 0x1;
    uint16_t target;
    if (bit11) {
        target = reg[R_PC] + sign_extend(instr & 0x7FF, 11);
    } else {
        uint16_t BASE_REG = (instr >> 6) & 0x7;
        target = reg[BASE_REG];
    }
    reg[R7] = reg[R_PC];
    reg[R_PC] = target;
}

void trap_fn(uint16_t instr, uint16_t* reg, uint16_t* memory) {
    reg[R7] = reg[R_PC];
    reg[R_PC] = mem_read(instr & 0xFF, memory);
}

void lea_fn(uint16_t instr, uint16_t* reg) {
    int DR_REG = (instr >> 0x9) & 0x7;
    int PCOFF9 = instr & 0x1FF;
    reg[DR_REG] = reg[R_PC] + sign_extend(PCOFF9, 9);
    update_flags(reg[DR_REG], reg);
}
