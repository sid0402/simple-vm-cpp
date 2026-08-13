#include <cstdint>
#include "lc3.h"

uint16_t mem_read(uint16_t address, uint16_t* memory) {
    return memory[address];
}


void mem_write(uint16_t address, uint16_t value, uint16_t* memory) {
    memory[address] = value;
}


uint16_t sign_extend(uint16_t input, int bit_count) {
    int last_bit = (input >> (bit_count - 1)) & 0x1;

    if (last_bit) {
        uint16_t bit_mask = 0xFFFF << bit_count;
        return input | bit_mask;
    }

    return input;
}


void update_flags(uint16_t output, uint16_t* reg) {
    if (output == 0) {
        reg[R_COND] = FL_ZER;
    } else if ((output >> 15) & 0x1) {
        reg[R_COND] = FL_NEG;
    } else {
        reg[R_COND] = FL_POS;
    }
}
