#include <cstdint>

#define MEMORY_MAX (1 << 16)
#define R_COUNT 10
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
    R_PC,
    R_COND
};

// instruction set
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


// condition flags
enum {
    FL_POS = 1 << 0,
    FL_ZER = 1 << 1,
    FL_NEG = 1 << 2,
};
