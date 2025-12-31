#include "../include/cpu.h"
#include "../include/memory.h"
#include "../include/instructions.h"
#include <stdio.h>
#include <assert.h>

int main() {
	printf("=== RISC-V Emulator Basic Tests ===\n\n");

	/* Test 1: Memory initialization */
	printf("Test 1: Memory operations...\n");
	memory_t *mem = memory_init(1024);
	assert(mem != NULL);
	assert(mem->size == 1024);

	/* Test write/read 8-bit */
	uint8_t val8;
	assert(memory_write8(mem, 100, 0x42) == MEM_OK);
	assert(memory_read8(mem, 100, &val8) == MEM_OK);
	assert(val8 == 0x42);

	/* Test write/read 32-bit */
	uint32_t val32;
	assert(memory_write32(mem, 200, 0xDEADBEEF) == MEM_OK);
	assert(memory_read32(mem, 200, &val32) == MEM_OK);
	assert(val32 == 0xDEADBEEF);

	memory_destroy(mem);
	printf("  OK Memory operations work\n");

	/* Test 2: CPU initialization */
	printf("Test 2: CPU initialization...\n");
	cpu_t *cpu = cpu_init();
	assert(cpu != NULL);
	assert(cpu->x[0] == 0);  /* x0 is always zero */
	assert(cpu->x[2] == STACK_TOP);  /* sp should be set */
	assert(cpu->running == 1);
	printf("  OK CPU initialization works\n");

	/* Test 3: Instruction decoding */
	printf("Test 3: Instruction decoding...\n");
	instruction_t instr;
	uint32_t add_instr = 0x003100B3;  /* add x1, x2, x3 */
	assert(cpu_decode(add_instr, &instr) == CPU_OK);
	assert(instr.format == INSTR_R_TYPE);
	assert(instr.opcode == 0x33);
	assert(instr.rd == 1);
	assert(instr.rs1 == 2);
	assert(instr.rs2 == 3);
	printf("  OK Instruction decoding works\n");

	/* Test 4: Sign extend */
	printf("Test 4: Sign extension...\n");
	assert(sign_extend(0xFF, 8) == -1);
	assert(sign_extend(0x7F, 8) == 127);
	assert(sign_extend(0x800, 12) == -2048);
	printf("  OK Sign extension works\n");

	cpu_destroy(cpu);

	printf("\n=== All basic emulator tests passed! ===\n");
	return 0;
}
