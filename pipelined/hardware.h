#pragma once

#include <cstdint>
#include "control_signals.h"

#define MEMORY_MAX (1 << 16)
#define R_COUNT 8
#define PC_START 0x3000

enum {
    R0,
    R1,
    R2,
    R3,
    R4,
    R5,
    R6,
    R7,
};

struct IFID {
    bool valid;
    uint16_t instruction;
    uint16_t PC_PLUS_ONE;
};

struct IDEX {
    bool valid;
    uint16_t PC_PLUS_ONE;
    uint16_t op;
    uint16_t SR1;
    uint16_t SR2;
    uint16_t SR1_VAL;
    uint16_t SR2_VAL;
    bool USE_SR1;
    bool USE_SR2;
    uint16_t DR;
    uint16_t offset;

    ALU_OPERATION alu_op;
    ALU_INPUT alu_input;
    PC_OP pc_op;
    WB_OP wb_op;
    bool REG_WRITE;
    bool MEM_READ;
    bool MEM_WRITE;
    bool CC_WRITE;
};

struct EXMEM {
    uint16_t DR;
    uint16_t ALU_RESULT;
    uint16_t PC_PLUS_ONE;
    uint16_t STORE_DATA;

    PC_OP pc_op;
    WB_OP wb_op;
    bool REG_WRITE;
    bool MEM_READ;
    bool MEM_WRITE;
    bool CC_WRITE;
};


struct MEMWB {
    uint16_t ALU_RESULT;
    uint16_t PC_PLUS_ONE;
    uint16_t DR;
    bool CC_WRITE;
    bool REG_WRITE;
};


uint16_t mem_read(uint16_t* memory, uint16_t address);

void mem_write(uint16_t* memory, uint16_t address, uint16_t value);

uint16_t reg_read(uint16_t* reg, uint16_t reg_id);

void reg_write(uint16_t* reg, uint16_t reg_id, uint16_t value);
