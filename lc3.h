#define MEMORY_MAX (1 << 16)
#define R_COUNT 10
#define PC_START 0x3000

int mem_read(int input);

int sign_extend(int input, int bit_count);
