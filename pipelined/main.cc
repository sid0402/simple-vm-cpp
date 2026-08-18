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

int main() {
    reg[R0] = 7;
    memory[PC_START] = 0x1425; // ADD R2, R0, #5
    memory[PC_START + 1] = 0xF025; // TRAP x25

    bool fetch_enabled = true;
    bool running = true;

    while (running) {

        IFID nextIFID{};
        IDEX nextIDEX{};
        EXMEM nextEXMEM{};
        MEMWB nextMEMWB{};

        // FETCH
        if (fetch_enabled) {
            const uint16_t instr = mem_read(memory, R_PC);
            R_PC++;
            nextIFID.valid = true;
            nextIFID.instruction = instr;
            nextIFID.PC_PLUS_ONE = R_PC;
        }

        // DECODE
        if (currIFID.valid) {
            const decoded_instruction decoded =
                decode_instruction(currIFID.instruction);
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
            const uint16_t first_operand =
                currIDEX.alu_a_input == ALU_A_INPUT::PC_PLUS_ONE
                    ? currIDEX.PC_PLUS_ONE
                    : currIDEX.SR1_VAL;
            const uint16_t second_operand =
                currIDEX.alu_b_input == ALU_B_INPUT::IMM
                    ? currIDEX.offset
                    : currIDEX.SR2_VAL;
            const uint16_t ALU_RESULT =
                alu(currIDEX.alu_op, first_operand, second_operand);
            nextEXMEM.valid = true;
            nextEXMEM.halt = currIDEX.halt;
            nextEXMEM.DR = currIDEX.DR;
            nextEXMEM.ALU_RESULT = ALU_RESULT;
            nextEXMEM.PC_PLUS_ONE = currIDEX.PC_PLUS_ONE;
            nextEXMEM.STORE_DATA = currIDEX.SR2_VAL;
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

        //WRITEBACK
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

        // CLOCK CYCLE
        currIFID = nextIFID;
        currIDEX = nextIDEX;
        currEXMEM = nextEXMEM;
        currMEMWB = nextMEMWB;
        clock_cycles++;

    };

}
