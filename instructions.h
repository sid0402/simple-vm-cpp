#include <cstdint>

void add_fn(uint16_t instr, uint16_t* reg);

void and_fn(uint16_t instr, uint16_t* reg);

void not_fn(uint16_t instr, uint16_t* reg);

void jmp_fn(uint16_t instr, uint16_t* reg);

void ld_fn(uint16_t instr, uint16_t* reg, uint16_t* memory);

void ldi_fn(uint16_t instr, uint16_t* reg, uint16_t* memory);

void ldr_fn(uint16_t instr, uint16_t* reg, uint16_t* memory);

void st_fn(uint16_t instr, uint16_t* reg, uint16_t* memory);

void sti_fn(uint16_t instr, uint16_t* reg, uint16_t* memory);

void str_fn(uint16_t instr, uint16_t* reg, uint16_t* memory);

void br_fn(uint16_t instr, uint16_t* reg);

void jsr_fn(uint16_t instr, uint16_t* reg);

void trap_fn(uint16_t instr, uint16_t* reg, uint16_t* memory);

void lea_fn(uint16_t instr, uint16_t* reg);
