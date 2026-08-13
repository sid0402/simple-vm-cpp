#include <cstdint>

void add_fn(uint16_t instr, uint16_t* reg);

void not_fn(uint16_t instr, uint16_t* reg);

void ld_fn(uint16_t instr, uint16_t* reg, uint16_t* memory);

void st_fn(uint16_t instr, uint16_t* reg, uint16_t* memory);

void br_fn(uint16_t instr, uint16_t* reg);

void jsr_fn(uint16_t instr, uint16_t* reg);
