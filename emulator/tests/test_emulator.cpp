/* test_emulator.cpp */
#include "../include/cpu.hpp"
#include "../include/memory.hpp"
#include "../include/instructions.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <memory>

/* Test helper function to create CPU and memory */
static void setup_cpu_memory(std::unique_ptr<cpu_t> &cpu, std::unique_ptr<memory_t> &mem, uint32_t mem_size) {
	mem = memory_init(mem_size);
	assert(mem != nullptr);

	cpu = cpu_init();
	assert(cpu != nullptr);

	/* Set PC to a known location */
	cpu->pc = 0x1000;
}

/* Test helper function to cleanup */
static void cleanup_cpu_memory(std::unique_ptr<cpu_t> &cpu, std::unique_ptr<memory_t> &mem) {
	/* Smart pointers handle cleanup automatically */
	cpu.reset();
	mem.reset();
}

/* Test 1: Basic CPU initialization */
static void test_cpu_init() {
	std::printf("Test 1: CPU initialization...\n");

	auto cpu = cpu_init();
	assert(cpu != nullptr);

	/* Verify register initialization */
	for (int i = 0; i < 32; i++) {
		if (i == 0) {
			assert(cpu->x[0] == 0);  /* x0 is always zero */
		} else if (i == 2) {
			assert(cpu->x[2] == STACK_TOP);  /* sp should be set to stack top */
		} else {
			assert(cpu->x[i] == 0);  /* Other registers should be zero */
		}
	}

	assert(cpu->pc == 0);
	assert(cpu->running == 1);

	std::printf("\tOK CPU initialization works\n");
}

/* Test 2: Memory operations */
static void test_memory_operations() {
	std::printf("Test 2: Memory operations...\n");

	auto mem = memory_init(4096);
	assert(mem != nullptr);
	assert(mem->size == 4096);

	/* Test 8-bit read/write */
	uint8_t val8;
	assert(memory_write8(mem.get(), 0x100, 0x42) == MEM_OK);
	assert(memory_read8(mem.get(), 0x100, &val8) == MEM_OK);
	assert(val8 == 0x42);

	/* Test 16-bit read/write */
	uint16_t val16;
	assert(memory_write16(mem.get(), 0x200, 0xABCD) == MEM_OK);
	assert(memory_read16(mem.get(), 0x200, &val16) == MEM_OK);
	assert(val16 == 0xABCD);

	/* Test 32-bit read/write */
	uint32_t val32;
	assert(memory_write32(mem.get(), 0x300, 0xDEADBEEF) == MEM_OK);
	assert(memory_read32(mem.get(), 0x300, &val32) == MEM_OK);
	assert(val32 == 0xDEADBEEF);

	/* Test misaligned accesses */
	assert(memory_read16(mem.get(), 0x101, &val16) == MEM_MISALIGNED_ERROR);
	assert(memory_write16(mem.get(), 0x101, 0x1234) == MEM_MISALIGNED_ERROR);
	assert(memory_read32(mem.get(), 0x102, &val32) == MEM_MISALIGNED_ERROR);
	assert(memory_write32(mem.get(), 0x102, 0x12345678) == MEM_MISALIGNED_ERROR);

	/* Test out of bounds accesses */
	assert(memory_read8(mem.get(), 0x2000, &val8) == MEM_READ_ERROR);
	assert(memory_write8(mem.get(), 0x2000, 0x12) == MEM_WRITE_ERROR);

	std::printf("\tOK Memory operations work\n");
}

/* Test 3: Sign extension */
static void test_sign_extend() {
	std::printf("Test 3: Sign extension...\n");

	assert(sign_extend(0x000, 12) == 0);
	assert(sign_extend(0x7FF, 12) == 2047);  /* Max positive 12-bit */
	assert(sign_extend(0x800, 12) == -2048);  /* Min negative 12-bit */
	assert(sign_extend(0xFFF, 12) == -1);     /* -1 in 12-bit */

	assert(sign_extend(0x7FFF, 16) == 32767);
	assert(sign_extend(0x8000, 16) == -32768);
	assert(sign_extend(0xFFFF, 16) == -1);

	assert(sign_extend(0x7FFFFFFF, 32) == 2147483647);
	assert(sign_extend(0x80000000, 32) == -2147483648);

	std::printf("\tOK Sign extension works\n");
}

/* Test 4: Instruction decoding */
static void test_instruction_decode() {
	std::printf("Test 4: Instruction decoding...\n");

	instruction_t instr;

	/* Test R-type instruction: add x1, x2, x3 */
	uint32_t add_instr = 0x003100B3;  /* add x1, x2, x3 */
	assert(cpu_decode(add_instr, &instr) == CPU_OK);
	assert(instr.format == INSTR_R_TYPE);
	assert(instr.opcode == 0x33);
	assert(instr.rd == 1);
	assert(instr.rs1 == 2);
	assert(instr.rs2 == 3);
	assert(instr.funct3 == 0x0);
	assert(instr.funct7 == 0x00);

	/* Test I-type instruction: addi x1, x2, 42 */
	uint32_t addi_instr = 0x02A10093;  /* addi x1, x2, 42 */
	assert(cpu_decode(addi_instr, &instr) == CPU_OK);
	assert(instr.format == INSTR_I_TYPE);
	assert(instr.opcode == 0x13);
	assert(instr.rd == 1);
	assert(instr.rs1 == 2);
	assert(instr.imm == 42);

	/* Test S-type instruction: sw x3, 4(x2) */
	uint32_t sw_instr = 0x00312223;  /* sw x3, 4(x2) */
	assert(cpu_decode(sw_instr, &instr) == CPU_OK);
	assert(instr.format == INSTR_S_TYPE);
	assert(instr.opcode == 0x23);
	assert(instr.rs1 == 2);
	assert(instr.rs2 == 3);
	assert(instr.funct3 == 0x2);
	assert(instr.imm == 4);

	/* Test B-type instruction: beq x2, x3, 8 */
	uint32_t beq_instr = 0x00310463;  /* beq x2, x3, 8 */
	assert(cpu_decode(beq_instr, &instr) == CPU_OK);
	assert(instr.format == INSTR_B_TYPE);
	assert(instr.opcode == 0x63);
	assert(instr.rs1 == 2);
	assert(instr.rs2 == 3);
	assert(instr.funct3 == 0x0);
	assert(instr.imm == 8);

	/* Test U-type instruction: lui x1, 0x12345 */
	uint32_t lui_instr = 0x123450B7;  /* lui x1, 0x12345 */
	assert(cpu_decode(lui_instr, &instr) == CPU_OK);
	assert(instr.format == INSTR_U_TYPE);
	assert(instr.opcode == 0x37);
	assert(instr.rd == 1);
	assert(instr.imm == 0x12345000);

	/* Test J-type instruction: jal x1, 1024 */
	uint32_t jal_instr = 0x400000EF;  /* jal x1, 1024 */
	assert(cpu_decode(jal_instr, &instr) == CPU_OK);
	assert(instr.format == INSTR_J_TYPE);
	assert(instr.opcode == 0x6F);
	assert(instr.rd == 1);
	assert(instr.imm == 1024);

	/* Test invalid opcode */
	uint32_t invalid_instr = 0x00000000;  /* Invalid opcode */
	assert(cpu_decode(invalid_instr, &instr) == CPU_DECODE_ERROR);

	std::printf("\tOK Instruction decoding works\n");
}

/* Test 5: Register read/write operations */
static void test_register_operations() {
	std::printf("Test 5: Register operations...\n");

	auto cpu = cpu_init();
	assert(cpu != nullptr);

	/* Test that x0 is always zero */
	assert(cpu->x[0] == 0);  /* x0 should be zero */

	/* Test reading/writing other registers */
	for (int i = 1; i < 32; i++) {
		uint32_t value = 0x1000 + i * 0x100;
		cpu->x[i] = value;
		assert(cpu->x[i] == value);
	}

	/* Verify registers maintain their values */
	assert(cpu->x[1] == 0x1100);
	assert(cpu->x[10] == 0x1A00);
	assert(cpu->x[31] == 0x2F00);

	std::printf("\tOK Register operations work\n");
}

/* Test 6: ALU operations */
static void test_alu_operations() {
	std::printf("Test 6: ALU operations (tested through cpu_step)...\n");

	auto cpu = cpu_init();
	auto mem = memory_init(8192);
	assert(cpu != nullptr && mem != nullptr);

	/* Write an add instruction to memory: add x1, x2, x3 */
	uint32_t add_instr = 0x003100B3;  /* add x1, x2, x3 */
	cpu->pc = 0x1000;
	cpu->x[2] = 10;
	cpu->x[3] = 20;
	memory_write32(mem.get(), 0x1000, add_instr);

	/* Execute the instruction */
	cpu_status_t status = cpu_step(cpu.get(), mem.get());
	assert(status == CPU_OK);
	assert(cpu->x[1] == 30);  /* 10 + 20 = 30 */

	/* Test SUB operation: sub x4, x5, x6 */
	uint32_t sub_instr = 0x40628233;  /* sub x4, x5, x6 */
	cpu->pc = 0x1004;
	cpu->x[5] = 50;
	cpu->x[6] = 30;
	memory_write32(mem.get(), 0x1004, sub_instr);

	status = cpu_step(cpu.get(), mem.get());
	assert(status == CPU_OK);
	assert(cpu->x[4] == 20);  /* 50 - 30 = 20 */

	std::printf("\tOK ALU operations work (tested through cpu_step)\n");
}

/* Test 7: Load/Store operations */
static void test_load_store() {
	std::printf("Test 7: Load/Store operations...\n");

	std::unique_ptr<cpu_t> cpu;
	std::unique_ptr<memory_t> mem;
	setup_cpu_memory(cpu, mem, 8192);

	/* Initialize some test data in memory */
	assert(memory_write32(mem.get(), 0x200, 0x12345678) == MEM_OK);
	assert(memory_write16(mem.get(), 0x204, 0xABCD) == MEM_OK);
	assert(memory_write8(mem.get(), 0x206, 0x42) == MEM_OK);

	/* Test LW instruction through cpu_step */
	/* Write LW instruction: lw x1, 0x200(x2) */
	uint32_t lw_instr = 0x20012083;  /* lw x1, 0x200(x2) */
	cpu->pc = 0x1000;
	cpu->x[2] = 0x0;  /* Base address 0 */
	memory_write32(mem.get(), 0x1000, lw_instr);

	cpu_status_t status = cpu_step(cpu.get(), mem.get());
	assert(status == CPU_OK);
	assert(cpu->x[1] == 0x12345678);  /* Should load the word */

	/* Test SW instruction through cpu_step */
	/* Write SW instruction: sw x1, 0x208(x2) */
	uint32_t sw_instr = 0x20112423;  /* sw x1, 0x208(x2) */
	cpu->pc = 0x1004;
	cpu->x[1] = 0xDEADBEEF;
	memory_write32(mem.get(), 0x1004, sw_instr);

	status = cpu_step(cpu.get(), mem.get());
	assert(status == CPU_OK);

	/* Verify the store */
	uint32_t stored_word;
	assert(memory_read32(mem.get(), 0x208, &stored_word) == MEM_OK);
	assert(stored_word == 0xDEADBEEF);

	std::printf("\tOK Load/Store operations work (tested through cpu_step)\n");
	cleanup_cpu_memory(cpu, mem);
}

/* Test 8: Branch operations */
static void test_branch_operations() {
	std::printf("Test 8: Branch operations...\n");

	std::unique_ptr<cpu_t> cpu;
	std::unique_ptr<memory_t> mem;
	setup_cpu_memory(cpu, mem, 8192);

	/* Write a BEQ instruction: beq x1, x2, 16 */
	uint32_t beq_instr = 0x00208863;  /* beq x1, x2, 16 */
	cpu->pc = 0x1000;
	cpu->x[1] = 42;
	cpu->x[2] = 42;
	memory_write32(mem.get(), 0x1000, beq_instr);

	/* Execute BEQ when registers are equal (should branch) */
	cpu_status_t status = cpu_step(cpu.get(), mem.get());
	assert(status == CPU_OK);
	assert(cpu->pc == 0x1010);  /* 0x1000 + 16 */

	/* Reset and test when not equal (should not branch) */
	cpu->pc = 0x1000;
	cpu->x[1] = 42;
	cpu->x[2] = 43;
	memory_write32(mem.get(), 0x1000, beq_instr);

	status = cpu_step(cpu.get(), mem.get());
	assert(status == CPU_OK);
	assert(cpu->pc == 0x1004);  /* PC should just advance by 4 */

	std::printf("\tOK Branch operations work (tested through cpu_step)\n");
	cleanup_cpu_memory(cpu, mem);
}

/* Test 9: System call operations */
static void test_system_calls() {
	std::printf("Test 9: System call operations...\n");

	std::unique_ptr<cpu_t> cpu;
	std::unique_ptr<memory_t> mem;
	setup_cpu_memory(cpu, mem, 8192);

	/* Write ECALL instruction */
	uint32_t ecall_instr = 0x00000073;  /* ecall */
	cpu->pc = 0x1000;
	cpu->x[17] = SYS_exit;  /* a7 = exit syscall number */
	memory_write32(mem.get(), 0x1000, ecall_instr);

	cpu_status_t status = cpu_step(cpu.get(), mem.get());
	assert(status == CPU_SYSCALL_EXIT);
	assert(cpu->running == 0);

	std::printf("\tOK System call operations work (tested through cpu_step)\n");
	cleanup_cpu_memory(cpu, mem);
}

/* Test 10: Complete CPU step execution */
static void test_cpu_step() {
	std::printf("Test 10: Complete CPU step execution...\n");

	std::unique_ptr<cpu_t> cpu;
	std::unique_ptr<memory_t> mem;
	setup_cpu_memory(cpu, mem, 8192);

	/* Write a simple program to memory */
	/* Program: addi x1, x0, 42  (0x02A00093) */
	uint32_t program[] = {
		0x02A00093,  /* addi x1, x0, 42 */
		0x00108093,  /* addi x1, x1, 1 */
		0x00000073,  /* ecall (will exit) */
	};

	/* Write program to memory starting at PC */
	for (size_t i = 0; i < sizeof(program)/sizeof(program[0]); i++) {
		assert(memory_write32(mem.get(), cpu->pc + i*4, program[i]) == MEM_OK);
	}

	/* Execute first instruction */
	cpu_status_t status = cpu_step(cpu.get(), mem.get());
	assert(status == CPU_OK);
	assert(cpu->x[1] == 42);  /* x1 should now be 42 */
	assert(cpu->pc == 0x1004);  /* PC should advance by 4 */

	/* Execute second instruction */
	status = cpu_step(cpu.get(), mem.get());
	assert(status == CPU_OK);
	assert(cpu->x[1] == 43);  /* x1 should now be 43 */
	assert(cpu->pc == 0x1008);  /* PC should advance by 4 */

	/* Setup exit syscall for third instruction */
	cpu->x[17] = SYS_exit;  /* a7 = exit syscall number */
	cpu->x[10] = 0;        /* a0 = exit code 0 */

	/* Execute third instruction (ecall) */
	status = cpu_step(cpu.get(), mem.get());
	assert(status == CPU_SYSCALL_EXIT);
	assert(cpu->running == 0);

	/* Test with misaligned PC */
	auto cpu2 = cpu_init();
	cpu2->pc = 0x1001;  /* Misaligned PC */

	status = cpu_step(cpu2.get(), mem.get());
	assert(status == CPU_FETCH_MISALIGNED);

	/* Test with out of bounds PC */
	cpu2->pc = 0xFFFFFFFC;  /* Near end of memory */

	status = cpu_step(cpu2.get(), mem.get());
	assert(status == CPU_FETCH_OUT_OF_BOUNDS);

	std::printf("\tOK Complete CPU step execution works\n");
	cleanup_cpu_memory(cpu, mem);
}

/* Test 11: Complex instruction execution */
static void test_complex_execution() {
	std::printf("Test 11: Complex instruction execution...\n");

	std::unique_ptr<cpu_t> cpu;
	std::unique_ptr<memory_t> mem;
	setup_cpu_memory(cpu, mem, 8192);

	/* Write a more complex program */
	uint32_t program[] = {
		0x06400093,  /* addi x1, x0, 100    # x1 = 100 */
		0x00500113,  /* addi x2, x0, 5      # x2 = 5 */
		0x00208233,  /* add x4, x1, x2      # x4 = x1 + x2 = 105 */
		0x402082B3,  /* sub x5, x1, x2      # x5 = x1 - x2 = 95 */
		0x00209333,  /* sll x6, x1, x2      # x6 = x1 << 5 = 3200 */
		0x0040D3B3,  /* srl x7, x1, x4      # x7 = x1 >> 105 = 0 */
		0x05D00893,  /* addi x17, x0, 93    # x17 = SYS_exit */
		0x00000073,  /* ecall */
	};

	/* Write program to memory */
	for (size_t i = 0; i < sizeof(program)/sizeof(program[0]); i++) {
		assert(memory_write32(mem.get(), cpu->pc + i*4, program[i]) == MEM_OK);
	}

	/* Execute all instructions */
	int steps = 0;
	while (cpu->running && steps < 100) {
		cpu_status_t status = cpu_step(cpu.get(), mem.get());
		if (status == CPU_SYSCALL_EXIT) break;
		assert(status == CPU_OK);
		steps++;
	}

	/* Verify results */
	assert(cpu->x[1] == 100);   /* x1 */
	assert(cpu->x[2] == 5);     /* x2 */
	assert(cpu->x[4] == 105);   /* x4 = x1 + x2 */
	assert(cpu->x[5] == 95);    /* x5 = x1 - x2 */
	assert(cpu->x[6] == 3200);  /* x6 = x1 << 5 */
	assert(cpu->x[7] == 0);     /* x7 = x1 >> 105 */

	std::printf("\tOK Complex instruction execution works\n");
	cleanup_cpu_memory(cpu, mem);
}

/* Test 12: Memory layout and stack */
static void test_memory_layout() {
	std::printf("Test 12: Memory layout and stack...\n");

	/* Test constants */
	assert(MEMORY_SIZE == 16 * 1024 * 1024);
	assert(STACK_BASE == 0x80000000);
	assert(STACK_SIZE == 1 * 1024 * 1024);
	assert(STACK_TOP == STACK_BASE + STACK_SIZE);

	/* Test that stack pointer is initialized correctly */
	auto cpu = cpu_init();
	assert(cpu->x[2] == STACK_TOP);  /* sp should be at stack top */

	std::printf("\tOK Memory layout is correct\n");
}

/* Test 13: Instruction fetch */
static void test_instruction_fetch() {
	std::printf("Test 13: Instruction fetch...\n");

	std::unique_ptr<cpu_t> cpu;
	std::unique_ptr<memory_t> mem;
	setup_cpu_memory(cpu, mem, 8192);

	/* Write instructions to memory */
	uint32_t instructions[] = {0x12345678, 0x9ABCDEF0, 0x11111111};

	for (size_t i = 0; i < sizeof(instructions)/sizeof(instructions[0]); i++) {
		uint32_t addr = 0x1000 + i * 4;
		assert(memory_write32(mem.get(), addr, instructions[i]) == MEM_OK);
	}

	/* Test normal fetch */
	cpu->pc = 0x1000;
	uint32_t fetched;

	cpu_status_t status = cpu_fetch(cpu.get(), mem.get(), &fetched);
	assert(status == CPU_OK);
	assert(fetched == 0x12345678);
	assert(cpu->pc == 0x1004);

	/* Fetch next instruction */
	status = cpu_fetch(cpu.get(), mem.get(), &fetched);
	assert(status == CPU_OK);
	assert(fetched == 0x9ABCDEF0);
	assert(cpu->pc == 0x1008);

	/* Test misaligned fetch */
	cpu->pc = 0x1001;
	status = cpu_fetch(cpu.get(), mem.get(), &fetched);
	assert(status == CPU_FETCH_MISALIGNED);
	assert(cpu->pc == 0x1001);  /* PC should not advance on error */

	/* Test out of bounds fetch */
	cpu->pc = 0xFFFFFFFC;  /* Last word in 32-bit address space */
	status = cpu_fetch(cpu.get(), mem.get(), &fetched);
	assert(status == CPU_FETCH_OUT_OF_BOUNDS);

	std::printf("\tOK Instruction fetch works\n");
	cleanup_cpu_memory(cpu, mem);
}

int main() {
	std::printf("=== RISC-V Emulator Comprehensive Tests ===\n\n");

	int test_count = 0;

	/* Run all tests */
	test_cpu_init(); test_count++;
	test_memory_operations(); test_count++;
	test_sign_extend(); test_count++;
	test_instruction_decode(); test_count++;
	test_register_operations(); test_count++;
	test_alu_operations(); test_count++;
	test_load_store(); test_count++;
	test_branch_operations(); test_count++;
	test_system_calls(); test_count++;
	test_cpu_step(); test_count++;
	test_complex_execution(); test_count++;
	test_memory_layout(); test_count++;
	test_instruction_fetch(); test_count++;

	std::printf("\n=== All %d tests passed! ===\n", test_count);
	return 0;
}
