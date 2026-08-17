#include "control_signals.h"
#include "instructions.h"

namespace {

uint16_t sign_extend(uint16_t input, uint16_t bit_count) {
    uint16_t value_mask = (1u << bit_count) - 1;
    input &= value_mask;

    if ((input >> (bit_count - 1)) & 0x1) {
        return input | static_cast<uint16_t>(~value_mask);
    }

    return input;
}

} // namespace

decoded_instruction decode_instruction(uint16_t instruction) {
    const uint16_t op = (instruction >> 12) & 0xF;

    decoded_instruction decoded{
        {
            ALU_OPERATION::PASS,
            ALU_INPUT::REG,
            IMMEDIATE_FORMAT::NONE,
            PC_OP::SEQ,
            WB_OP::ALU,
            false, // REG_WRITE
            false, // MEM_READ
            false, // MEM_WRITE
            false  // CC_WRITE
        },
        op, //op
        0, //src1
        0, //src2
        0, //dest
        0,//imm
        false, //uses_src1
        false //uses_src2
    };

    switch (op) {
        case OP_ADD: {
            const bool uses_immediate = (instruction >> 5) & 0x1;

            decoded.src1 = (instruction >> 6) & 0x7;
            decoded.dest = (instruction >> 9) & 0x7;
            decoded.uses_src1 = true;
            decoded.uses_src2 = !uses_immediate;

            if (uses_immediate) {
                decoded.immediate = sign_extend(instruction & 0x1F, 5);
                decoded.control.immediate_format = IMMEDIATE_FORMAT::IMM5;
                decoded.control.alu_input = ALU_INPUT::IMM;
            } else {
                decoded.src2 = instruction & 0x7;
                decoded.control.alu_input = ALU_INPUT::REG;
            }

            decoded.control.alu_op = ALU_OPERATION::ADD;
            decoded.control.REG_WRITE = true;
            decoded.control.CC_WRITE = true;
            break;
        }

        default:
            break;
    }

    return decoded;
}
