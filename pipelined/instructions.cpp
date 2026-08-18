#include "control_signals.h"
#include "instructions.h"

namespace {

constexpr uint16_t LINK_REGISTER = 7;

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
            ALU_A_INPUT::SRC1,
            ALU_B_INPUT::SRC2,
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
        0, //condition_mask
        false, //uses_src1
        false, //uses_src2
        false //halt
    };

    switch (op) {
        case OP_BR:
            decoded.immediate = sign_extend(instruction & 0x1FF, 9);
            decoded.condition_mask = (instruction >> 9) & 0x7;
            decoded.uses_src1 = false;
            decoded.uses_src2 = false;
            decoded.control.alu_op = ALU_OPERATION::ADD;
            decoded.control.alu_a_input = ALU_A_INPUT::PC_PLUS_ONE;
            decoded.control.alu_b_input = ALU_B_INPUT::IMM;
            decoded.control.immediate_format = IMMEDIATE_FORMAT::OFFSET9;
            decoded.control.pc_op = PC_OP::BRANCH;
            break;

        case OP_ADD: {
            const bool uses_immediate = (instruction >> 5) & 0x1;

            decoded.src1 = (instruction >> 6) & 0x7;
            decoded.dest = (instruction >> 9) & 0x7;
            decoded.uses_src1 = true;
            decoded.uses_src2 = !uses_immediate;

            if (uses_immediate) {
                decoded.immediate = sign_extend(instruction & 0x1F, 5);
                decoded.control.immediate_format = IMMEDIATE_FORMAT::IMM5;
                decoded.control.alu_b_input = ALU_B_INPUT::IMM;
            } else {
                decoded.src2 = instruction & 0x7;
                decoded.control.alu_b_input = ALU_B_INPUT::SRC2;
            }

            decoded.control.alu_op = ALU_OPERATION::ADD;
            decoded.control.REG_WRITE = true;
            decoded.control.CC_WRITE = true;
            break;
        }

        case OP_AND: {
            const bool uses_immediate = (instruction >> 5) & 0x1;

            decoded.src1 = (instruction >> 6) & 0x7;
            decoded.dest = (instruction >> 9) & 0x7;
            decoded.uses_src1 = true;
            decoded.uses_src2 = !uses_immediate;

            if (uses_immediate) {
                decoded.immediate = sign_extend(instruction & 0x1F, 5);
                decoded.control.immediate_format = IMMEDIATE_FORMAT::IMM5;
                decoded.control.alu_b_input = ALU_B_INPUT::IMM;
            } else {
                decoded.src2 = instruction & 0x7;
                decoded.control.alu_b_input = ALU_B_INPUT::SRC2;
            }

            decoded.control.alu_op = ALU_OPERATION::AND;
            decoded.control.REG_WRITE = true;
            decoded.control.CC_WRITE = true;
            break;
        }

        case OP_LD:
            decoded.dest = (instruction >> 9) & 0x7;
            decoded.immediate = sign_extend(instruction & 0x1FF, 9);
            decoded.uses_src1 = false;
            decoded.uses_src2 = false;
            decoded.control.alu_op = ALU_OPERATION::ADD;
            decoded.control.alu_a_input = ALU_A_INPUT::PC_PLUS_ONE;
            decoded.control.alu_b_input = ALU_B_INPUT::IMM;
            decoded.control.immediate_format = IMMEDIATE_FORMAT::OFFSET9;
            decoded.control.wb_op = WB_OP::MEM;
            decoded.control.REG_WRITE = true;
            decoded.control.MEM_READ = true;
            decoded.control.CC_WRITE = true;
            break;

        case OP_ST:
            decoded.src2 = (instruction >> 9) & 0x7;
            decoded.immediate = sign_extend(instruction & 0x1FF, 9);
            decoded.uses_src1 = false;
            decoded.uses_src2 = true;
            decoded.control.alu_op = ALU_OPERATION::ADD;
            decoded.control.alu_a_input = ALU_A_INPUT::PC_PLUS_ONE;
            decoded.control.alu_b_input = ALU_B_INPUT::IMM;
            decoded.control.immediate_format = IMMEDIATE_FORMAT::OFFSET9;
            decoded.control.MEM_WRITE = true;
            break;

        case OP_LDR:
            decoded.src1 = (instruction >> 6) & 0x7;
            decoded.dest = (instruction >> 9) & 0x7;
            decoded.immediate = sign_extend(instruction & 0x3F, 6);
            decoded.uses_src1 = true;
            decoded.uses_src2 = false;
            decoded.control.alu_op = ALU_OPERATION::ADD;
            decoded.control.alu_a_input = ALU_A_INPUT::SRC1;
            decoded.control.alu_b_input = ALU_B_INPUT::IMM;
            decoded.control.immediate_format = IMMEDIATE_FORMAT::OFFSET6;
            decoded.control.wb_op = WB_OP::MEM;
            decoded.control.REG_WRITE = true;
            decoded.control.MEM_READ = true;
            decoded.control.CC_WRITE = true;
            break;

        case OP_STR:
            decoded.src1 = (instruction >> 6) & 0x7;
            decoded.src2 = (instruction >> 9) & 0x7;
            decoded.immediate = sign_extend(instruction & 0x3F, 6);
            decoded.uses_src1 = true;
            decoded.uses_src2 = true;
            decoded.control.alu_op = ALU_OPERATION::ADD;
            decoded.control.alu_a_input = ALU_A_INPUT::SRC1;
            decoded.control.alu_b_input = ALU_B_INPUT::IMM;
            decoded.control.immediate_format = IMMEDIATE_FORMAT::OFFSET6;
            decoded.control.MEM_WRITE = true;
            break;

        case OP_NOT:
            decoded.src1 = (instruction >> 6) & 0x7;
            decoded.dest = (instruction >> 9) & 0x7;
            decoded.uses_src1 = true;
            decoded.uses_src2 = false;
            decoded.control.alu_op = ALU_OPERATION::NOT;
            decoded.control.REG_WRITE = true;
            decoded.control.CC_WRITE = true;
            break;

        case OP_JSR: {
            const bool uses_pc_offset = (instruction >> 11) & 0x1;

            decoded.dest = LINK_REGISTER;
            decoded.control.pc_op = PC_OP::SUB;
            decoded.control.wb_op = WB_OP::PC;
            decoded.control.REG_WRITE = true;

            if (uses_pc_offset) {
                decoded.immediate = sign_extend(instruction & 0x7FF, 11);
                decoded.uses_src1 = false;
                decoded.uses_src2 = false;
                decoded.control.alu_op = ALU_OPERATION::ADD;
                decoded.control.alu_a_input = ALU_A_INPUT::PC_PLUS_ONE;
                decoded.control.alu_b_input = ALU_B_INPUT::IMM;
                decoded.control.immediate_format = IMMEDIATE_FORMAT::OFFSET11;
            } else {
                decoded.src1 = (instruction >> 6) & 0x7;
                decoded.uses_src1 = true;
                decoded.uses_src2 = false;
                decoded.control.alu_op = ALU_OPERATION::PASS;
                decoded.control.alu_a_input = ALU_A_INPUT::SRC1;
            }
            break;
        }

        case OP_JMP:
            decoded.src1 = (instruction >> 6) & 0x7;
            decoded.uses_src1 = true;
            decoded.uses_src2 = false;
            decoded.control.alu_op = ALU_OPERATION::PASS;
            decoded.control.alu_a_input = ALU_A_INPUT::SRC1;
            decoded.control.pc_op = PC_OP::JMP;
            break;

        case OP_LEA:
            decoded.dest = (instruction >> 9) & 0x7;
            decoded.immediate = sign_extend(instruction & 0x1FF, 9);
            decoded.uses_src1 = false;
            decoded.uses_src2 = false;
            decoded.control.alu_op = ALU_OPERATION::ADD;
            decoded.control.alu_a_input = ALU_A_INPUT::PC_PLUS_ONE;
            decoded.control.alu_b_input = ALU_B_INPUT::IMM;
            decoded.control.immediate_format = IMMEDIATE_FORMAT::OFFSET9;
            decoded.control.REG_WRITE = true;
            decoded.control.CC_WRITE = true;
            break;

        case OP_TRAP:
            if ((instruction & 0xFF) == 0x25) {
                decoded.halt = true;
            }
            break;

        default:
            break;
    }

    return decoded;
}
