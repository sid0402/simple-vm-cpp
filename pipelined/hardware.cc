#include "hardware.h"
#include "cstdint"

uint16_t mem_read(uint16_t* memory, uint16_t address) {
    return memory[address];
};

void mem_write(uint16_t* memory, uint16_t address, uint16_t value) {
    memory[address] = value;
};

uint16_t reg_read(uint16_t* reg, uint16_t reg_id) {
    return reg[reg_id];
};

void reg_write(uint16_t* reg, uint16_t reg_id, uint16_t value) {
    reg[reg_id] = value;
};

void update_cc(uint16_t value, uint16_t* cc) {
    if (value == 0) {
        *cc = FL_ZER;
    } else if (value & 0x8000) {
        *cc = FL_NEG;
    } else {
        *cc = FL_POS;
    }
}

uint16_t alu(ALU_OPERATION alu_op, uint16_t src1, uint16_t src2) {
    if (alu_op == ALU_OPERATION::ADD) {
        return src1 + src2;
    } else if (alu_op == ALU_OPERATION::AND) {
        return src1 & src2;
    } else if (alu_op == ALU_OPERATION::NOT) {
        return src1 ^ 0xFFFF;
    } else {
        return 0; // interrupt
    }
}
