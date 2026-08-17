#include <cstdint>
#include "hardware.h"
#include "instructions.h"

uint16_t reg[R_COUNT];
uint16_t memory[MEMORY_MAX];
uint16_t R_PC = PC_START;
uint16_t R_CC;

IFID currIFID;
IFID nextIFID;

IDEX currIDEX;
IDEX nextIDEX;

EXMEM currEXMEM;
EXMEM nextEXMEM;

MEMWB currMEMWB;
MEMWB nextMEMWB;

int clock_cycles = 0;

int main() {

    while (true) {

        // FETCH
        uint16_t instr = mem_read(memory, R_PC);
        R_PC++;
        nextIFID.valid = true;
        nextIFID.instruction = instr;
        nextIFID.PC_PLUS_ONE = R_PC;

        // DECODE
        nextIDEX = {};

        if (currIFID.valid) {
            const decoded_instruction decoded =
                decode_instruction(currIFID.instruction);
            const controls& decode_controls = decoded.control;

            nextIDEX.valid = true;
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
            nextIDEX.alu_op = decode_controls.alu_op;
            nextIDEX.alu_input = decode_controls.alu_input;
            nextIDEX.pc_op = decode_controls.pc_op;
            nextIDEX.wb_op = decode_controls.wb_op;
            nextIDEX.REG_WRITE = decode_controls.REG_WRITE;
            nextIDEX.MEM_WRITE = decode_controls.MEM_WRITE;
            nextIDEX.MEM_READ = decode_controls.MEM_READ;
            nextIDEX.CC_WRITE = decode_controls.CC_WRITE;
        }

        
    };

}
