#include "lc3.h"
#include "instructions.h"
#include "utils.h"
#include <cstdio>
#include <cstdlib>
#include <cstdint>

uint16_t reg[R_COUNT];
uint16_t memory[MEMORY_MAX];

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

        uint16_t instr = mem_read(reg[R_PC]++, memory);
        int op = (instr >> 12) & 0xF;

        switch (op) {
            case OP_BR:
                br_fn(instr, reg);
                break;

            case OP_ADD:
                add_fn(instr, reg);
                break;

            case OP_LD:
                ld_fn(instr, reg, memory);
                break;

            case OP_ST:
                st_fn(instr, reg, memory);
                break;

            case OP_JSR:
                jsr_fn(instr, reg);
                break;

            case OP_AND:
                and_fn(instr, reg);
                break;

            case OP_LDR:
                ldr_fn(instr, reg, memory);
                break;

            case OP_STR:
                str_fn(instr, reg, memory);
                break;

            case OP_NOT:
                not_fn(instr, reg);
                break;

            case OP_LDI:
                ldi_fn(instr, reg, memory);
                break;

            case OP_STI:
                sti_fn(instr, reg, memory);
                break;

            case OP_JMP:
                jmp_fn(instr, reg);
                break;

            case OP_LEA:
                lea_fn(instr, reg);
                break;

            case OP_TRAP:
                trap_fn(instr, reg, memory);
                if ((instr & 0xFF) == 0x25) {
                    running = 0;
                }
                break;

            case OP_RES:
            case OP_RTI:
            default:
                fprintf(stderr, "bad opcode: 0x%04X\n", instr);
                return EXIT_FAILURE;
        }
    }
}
