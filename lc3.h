#define MEMORY_MAX (1 << 16)
#define R_COUNT 10
#define PC_START 0x3000
#include <cstdint>

uint16_t mem_read(uint16_t address);

uint16_t sign_extend(uint16_t input, int bit_count);
