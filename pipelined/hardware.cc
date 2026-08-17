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
