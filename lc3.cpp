#include "lc3.h"
#include <cstdio>
#include <cstdlib>
#include <cstdint>


// memory
uint16_t memory[MEMORY_MAX];
// 2^16 memory locations


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

uint16_t reg[R_COUNT];


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


uint16_t mem_read(uint16_t address) {
    return memory[address];
}


void mem_write(uint16_t address, uint16_t value) {
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


void update_flags(uint16_t output) {
    if (output == 0) {
        reg[R_COND] = FL_ZER;
    } else if ((output >> 15) & 0x1) {
        reg[R_COND] = FL_NEG;
    } else {
        reg[R_COND] = FL_POS;
    }
}


void add_fn(uint16_t instr) {
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

    update_flags(reg[DR_REG]);
}


void not_fn(uint16_t instr) {
    int DR_REG = (instr >> 9) & 0x7;
    int SR_REG = (instr >> 6) & 0x7;

    reg[DR_REG] = reg[SR_REG] ^ 0xFFFF;

    update_flags(reg[DR_REG]);
}


void ld_fn(uint16_t instr) {
    int DR_REG = (instr >> 9) & 0x7;
    uint16_t PCOFF9 = sign_extend(instr & 0x1FF, 9);

    reg[DR_REG] = mem_read(reg[R_PC] + PCOFF9);

    update_flags(reg[DR_REG]);
}


void st_fn(uint16_t instr) {
    int SR_REG = (instr >> 9) & 0x7;
    uint16_t PCOFF9 = sign_extend(instr & 0x1FF, 9);

    mem_write(reg[R_PC] + PCOFF9, reg[SR_REG]);
}


void br_fn(uint16_t instr) {
    int n = ((instr >> 11) & 0x1) && (reg[R_COND] == FL_NEG);
    int z = ((instr >> 10) & 0x1) && (reg[R_COND] == FL_ZER);
    int p = ((instr >> 9) & 0x1) && (reg[R_COND] == FL_POS);

    if (n || z || p) {
        uint16_t PCOFF9 = sign_extend(instr & 0x1FF, 9);
        reg[R_PC] = reg[R_PC] + PCOFF9;
    }
}


void jsr_fn(uint16_t instr) {
    uint16_t PCOFF11 = instr & 0x7FF;

    reg[R7] = reg[R_PC];
    reg[R_PC] = reg[R_PC] + sign_extend(PCOFF11, 11);
}


uint16_t swap16(uint16_t x) {
    return (x << 8) | (x >> 8);
}


void read_image_file(FILE* file) {
    // origin tells us where in memory to place the image
    uint16_t origin;

    fread(&origin, sizeof(origin), 1, file);
    origin = swap16(origin);

    // maximum number of words we can read
    uint16_t* p = memory + origin;
    size_t max_read = MEMORY_MAX - origin;

    size_t read = fread(
        p,
        sizeof(uint16_t),
        max_read,
        file
    );

    // LC-3 object files store words in big endian
    while (read-- > 0) {
        *p = swap16(*p);
        ++p;
    }
}


int read_image(const char* image_path) {
    FILE* file = fopen(image_path, "rb");

    if (!file) {
        return 0;
    }

    read_image_file(file);
    fclose(file);

    return 1;
}


int main(int argc, const char* argv[]) {

    if (argc < 2) {
        printf("lc3 [image-file1] ...\n");
        exit(2);
    }

    for (int j = 1; j < argc; ++j) {
        if (!read_image(argv[j])) {
            printf("failed to load image: %s\n", argv[j]);
            exit(1);
        }
    }

    reg[R_COND] = FL_ZER;
    reg[R_PC] = PC_START;

    int running = 1;

    while (running) {

        uint16_t instr = mem_read(reg[R_PC]++);
        int op = (instr >> 12) & 0xF;

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

            case OP_TRAP:
                if ((instr & 0xFF) == 0x25) {
                    running = 0;
                }
                break;
        }
    }
}
