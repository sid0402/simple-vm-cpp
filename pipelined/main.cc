#include <cstdint>
#include "hardware.h"
#include "instructions.h"

uint16_t reg[R_COUNT];
uint16_t memory[MEMORY_MAX];
uint16_t R_PC = PC_START;
uint16_t R_CC = FL_ZER;

IFID currIFID;
IDEX currIDEX;
EXMEM currEXMEM;
MEMWB currMEMWB;

int clock_cycles = 0;

bool has_register_dependency(
    bool source_used,
    uint16_t source,
    bool producer_valid,
    bool producer_reg_write,
    uint16_t producer_dest
) {
    return source_used &&
           producer_valid &&
           producer_reg_write &&
           source == producer_dest;
}

int main() {
    memory[PC_START + 0x00] = 0x5020; // AND R0, R0, #0
    memory[PC_START + 0x01] = 0x1025; // ADD R0, R0, #5
    memory[PC_START + 0x02] = 0x923F; // NOT R1, R0
    memory[PC_START + 0x03] = 0x1261; // ADD R1, R1, #1
    memory[PC_START + 0x04] = 0x1422; // ADD R2, R0, #2
    memory[PC_START + 0x05] = 0x341A; // ST R2, DATA
    memory[PC_START + 0x06] = 0x2619; // LD R3, DATA
    memory[PC_START + 0x07] = 0x18E1; // ADD R4, R3, #1
    memory[PC_START + 0x08] = 0xEA17; // LEA R5, DATA
    memory[PC_START + 0x09] = 0x7941; // STR R4, R5, #1
    memory[PC_START + 0x0A] = 0x6D41; // LDR R6, R5, #1
    memory[PC_START + 0x0B] = 0x5DAF; // AND R6, R6, #15
    memory[PC_START + 0x0C] = 0x1DB8; // ADD R6, R6, #-8
    memory[PC_START + 0x0D] = 0x0402; // BRz BRANCH_TAKEN
    memory[PC_START + 0x0E] = 0x1FE1; // ADD R7, R7, #1 (flush)
    memory[PC_START + 0x0F] = 0x1FE1; // ADD R7, R7, #1 (flush)
    memory[PC_START + 0x10] = 0x4807; // JSR SUBROUTINE_ONE
    memory[PC_START + 0x11] = 0xEA08; // LEA R5, SUBROUTINE_TWO
    memory[PC_START + 0x12] = 0x4140; // JSRR R5
    memory[PC_START + 0x13] = 0xEA02; // LEA R5, FINISH
    memory[PC_START + 0x14] = 0xC140; // JMP R5
    memory[PC_START + 0x15] = 0x102F; // ADD R0, R0, #15 (flush)
    memory[PC_START + 0x16] = 0xF025; // TRAP x25
    memory[PC_START + 0x17] = 0x0000; // Unused
    memory[PC_START + 0x18] = 0x1021; // ADD R0, R0, #1
    memory[PC_START + 0x19] = 0xC1C0; // RET
    memory[PC_START + 0x1A] = 0x1021; // ADD R0, R0, #1
    memory[PC_START + 0x1B] = 0xC1C0; // RET
    memory[PC_START + 0x20] = 0x0000; // DATA
    memory[PC_START + 0x21] = 0x0000; // DATA2

    bool fetch_enabled = true;
    bool running = true;

    while (running) {

        IFID nextIFID{};
        IDEX nextIDEX{};
        EXMEM nextEXMEM{};
        MEMWB nextMEMWB{};

        // WRITEBACK
        if (currMEMWB.valid) {
            if (currMEMWB.REG_WRITE) {
                reg_write(reg, currMEMWB.DR, currMEMWB.WRITEBACK_VALUE);
            }
            if (currMEMWB.CC_WRITE) {
                update_cc(currMEMWB.WRITEBACK_VALUE, &R_CC);
            }
            if (currMEMWB.halt) {
                running = false;
            }
        }

        decoded_instruction decoded{};
        if (currIFID.valid) {
            decoded = decode_instruction(currIFID.instruction);
        }

        bool load_use_stall = false;
        if (currIFID.valid &&
            currIDEX.valid &&
            currIDEX.MEM_READ &&
            currIDEX.REG_WRITE) {
            const bool src1_dependency =
                has_register_dependency(
                    decoded.uses_src1,
                    decoded.src1,
                    currIDEX.valid,
                    currIDEX.REG_WRITE,
                    currIDEX.DR
                );
            const bool src2_dependency =
                has_register_dependency(
                    decoded.uses_src2,
                    decoded.src2,
                    currIDEX.valid,
                    currIDEX.REG_WRITE,
                    currIDEX.DR
                );

            load_use_stall = src1_dependency || src2_dependency;
        }

        const bool condition_code_stall =
            currIFID.valid &&
            decoded.control.pc_op == PC_OP::BRANCH &&
            currIDEX.valid &&
            currIDEX.CC_WRITE;
        const bool decode_stall =
            load_use_stall || condition_code_stall;

        // FETCH
        if (fetch_enabled && !decode_stall) {
            const uint16_t instr = mem_read(memory, R_PC);
            R_PC++;
            nextIFID.valid = true;
            nextIFID.instruction = instr;
            nextIFID.PC_PLUS_ONE = R_PC;
        } else if (decode_stall) {
            nextIFID = currIFID;
        }

        // DECODE
        if (currIFID.valid && !decode_stall) {
            const controls& decode_controls = decoded.control;

            nextIDEX.valid = true;
            nextIDEX.halt = decoded.halt;
            nextIDEX.PC_PLUS_ONE = currIFID.PC_PLUS_ONE;
            nextIDEX.op = decoded.opcode;
            nextIDEX.SR1 = decoded.src1;
            nextIDEX.SR2 = decoded.src2;
            nextIDEX.USE_SR1 = decoded.uses_src1;
            nextIDEX.USE_SR2 = decoded.uses_src2;
            nextIDEX.SR1_VAL = decoded.uses_src1
                ? reg_read(reg, decoded.src1)
                : 0;
            nextIDEX.SR2_VAL = decoded.uses_src2
                ? reg_read(reg, decoded.src2)
                : 0;
            nextIDEX.DR = decoded.dest;
            nextIDEX.offset = decoded.immediate;
            nextIDEX.condition_mask = decoded.condition_mask;
            nextIDEX.alu_op = decode_controls.alu_op;
            nextIDEX.alu_a_input = decode_controls.alu_a_input;
            nextIDEX.alu_b_input = decode_controls.alu_b_input;
            nextIDEX.pc_op = decode_controls.pc_op;
            nextIDEX.wb_op = decode_controls.wb_op;
            nextIDEX.REG_WRITE = decode_controls.REG_WRITE;
            nextIDEX.MEM_WRITE = decode_controls.MEM_WRITE;
            nextIDEX.MEM_READ = decode_controls.MEM_READ;
            nextIDEX.CC_WRITE = decode_controls.CC_WRITE;
        }

        // EXECUTE
        if (currIDEX.valid) {
            uint16_t forwarded_src1 = currIDEX.SR1_VAL;
            uint16_t forwarded_src2 = currIDEX.SR2_VAL;

            const bool exmem_src1_dependency =
                has_register_dependency(
                    currIDEX.USE_SR1,
                    currIDEX.SR1,
                    currEXMEM.valid,
                    currEXMEM.REG_WRITE,
                    currEXMEM.DR
                );
            const bool exmem_src2_dependency =
                has_register_dependency(
                    currIDEX.USE_SR2,
                    currIDEX.SR2,
                    currEXMEM.valid,
                    currEXMEM.REG_WRITE,
                    currEXMEM.DR
                );

            const bool exmem_value_available =
                currEXMEM.wb_op != WB_OP::MEM;
            const uint16_t exmem_forward_value =
                currEXMEM.wb_op == WB_OP::PC
                    ? currEXMEM.PC_PLUS_ONE
                    : currEXMEM.ALU_RESULT;

            if (exmem_src1_dependency) {
                if (exmem_value_available) {
                    forwarded_src1 = exmem_forward_value;
                }
            } else if (has_register_dependency(
                           currIDEX.USE_SR1,
                           currIDEX.SR1,
                           currMEMWB.valid,
                           currMEMWB.REG_WRITE,
                           currMEMWB.DR
                       )) {
                forwarded_src1 = currMEMWB.WRITEBACK_VALUE;
            }

            if (exmem_src2_dependency) {
                if (exmem_value_available) {
                    forwarded_src2 = exmem_forward_value;
                }
            } else if (has_register_dependency(
                           currIDEX.USE_SR2,
                           currIDEX.SR2,
                           currMEMWB.valid,
                           currMEMWB.REG_WRITE,
                           currMEMWB.DR
                       )) {
                forwarded_src2 = currMEMWB.WRITEBACK_VALUE;
            }

            const uint16_t first_operand =
                currIDEX.alu_a_input == ALU_A_INPUT::PC_PLUS_ONE
                    ? currIDEX.PC_PLUS_ONE
                    : forwarded_src1;
            const uint16_t second_operand =
                currIDEX.alu_b_input == ALU_B_INPUT::IMM
                    ? currIDEX.offset
                    : forwarded_src2;
            const uint16_t ALU_RESULT =
                alu(currIDEX.alu_op, first_operand, second_operand);
            nextEXMEM.valid = true;
            nextEXMEM.halt = currIDEX.halt;
            nextEXMEM.DR = currIDEX.DR;
            nextEXMEM.ALU_RESULT = ALU_RESULT;
            nextEXMEM.PC_PLUS_ONE = currIDEX.PC_PLUS_ONE;
            nextEXMEM.STORE_DATA = forwarded_src2;
            nextEXMEM.pc_op = currIDEX.pc_op;
            nextEXMEM.wb_op = currIDEX.wb_op;
            nextEXMEM.REG_WRITE = currIDEX.REG_WRITE;
            nextEXMEM.MEM_READ = currIDEX.MEM_READ;
            nextEXMEM.MEM_WRITE = currIDEX.MEM_WRITE;
            nextEXMEM.CC_WRITE = currIDEX.CC_WRITE;

            if (currIDEX.pc_op == PC_OP::JMP ||
                currIDEX.pc_op == PC_OP::SUB) {
                R_PC = ALU_RESULT;
                nextIFID = {};
                nextIDEX = {};
            }

            if (currIDEX.pc_op == PC_OP::BRANCH &&
                (currIDEX.condition_mask & R_CC) != 0) {
                R_PC = ALU_RESULT;
                nextIFID = {};
                nextIDEX = {};
            }

            if (currIDEX.halt) {
                fetch_enabled = false;
                R_PC = currIDEX.PC_PLUS_ONE;
                nextIFID = {};
                nextIDEX = {};
            }
        }

        // MEMORY
        if (currEXMEM.valid) {
            uint16_t memory_data = 0;
            if (currEXMEM.MEM_READ) {
                memory_data = mem_read(memory, currEXMEM.ALU_RESULT);
            }
            if (currEXMEM.MEM_WRITE) {
                mem_write(memory, currEXMEM.ALU_RESULT, currEXMEM.STORE_DATA);
            }

            nextMEMWB.valid = true;
            nextMEMWB.halt = currEXMEM.halt;
            switch (currEXMEM.wb_op) {
                case WB_OP::ALU:
                    nextMEMWB.WRITEBACK_VALUE = currEXMEM.ALU_RESULT;
                    break;
                case WB_OP::MEM:
                    nextMEMWB.WRITEBACK_VALUE = memory_data;
                    break;
                case WB_OP::PC:
                    nextMEMWB.WRITEBACK_VALUE = currEXMEM.PC_PLUS_ONE;
                    break;
            }
            nextMEMWB.PC_PLUS_ONE = currEXMEM.PC_PLUS_ONE;
            nextMEMWB.DR = currEXMEM.DR;
            nextMEMWB.CC_WRITE = currEXMEM.CC_WRITE;
            nextMEMWB.REG_WRITE = currEXMEM.REG_WRITE;
        }

        // CLOCK CYCLE
        currIFID = nextIFID;
        currIDEX = nextIDEX;
        currEXMEM = nextEXMEM;
        currMEMWB = nextMEMWB;
        clock_cycles++;

    };

}
