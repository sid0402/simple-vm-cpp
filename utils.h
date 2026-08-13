#include <cstdint>

uint16_t mem_read(uint16_t address, uint16_t* memory);

void mem_write(uint16_t address, uint16_t value, uint16_t* memory);

uint16_t sign_extend(uint16_t input, int bit_count);

void update_flags(uint16_t output, uint16_t* reg);
