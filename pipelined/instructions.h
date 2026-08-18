#pragma once

#include <cstdint>
#include "control_signals.h"

enum {
    OP_BR   = 0x0,
    OP_ADD  = 0x1,
    OP_LD   = 0x2,
    OP_ST   = 0x3,
    OP_JSR  = 0x4,
    OP_AND  = 0x5,
    OP_LDR  = 0x6,
    OP_STR  = 0x7,
    OP_RTI  = 0x8,
    OP_NOT  = 0x9,
    OP_LDI  = 0xA,
    OP_STI  = 0xB,
    OP_JMP  = 0xC,
    OP_RES  = 0xD,
    OP_LEA  = 0xE,
    OP_TRAP = 0xF
};

struct decoded_instruction {
    controls control;
    uint16_t opcode;
    uint16_t src1;
    uint16_t src2;
    uint16_t dest;
    uint16_t immediate;
    uint16_t condition_mask;
    bool uses_src1;
    bool uses_src2;
    bool halt;
};

decoded_instruction decode_instruction(uint16_t instruction);
