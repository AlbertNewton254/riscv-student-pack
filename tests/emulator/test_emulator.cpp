/* test_emulator.cpp */
#include "../include/cpu.hpp"
#include "../include/memory.hpp"
#include "../include/instructions.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <memory>

/* Test 1: Basic CPU initialization */
static void test_cpu_init() {
	std::printf("Test 1: CPU initialization...\n");

	CPU cpu;

	/* Verify register initialization */
	for (int i = 0; i < 32; i++) {
		if (i == 0) {
			assert(cpu.get_register(0) == 0);  /* x0 is always zero */
		} else if (i == 2) {
			assert(cpu.get_register(2) == STACK_TOP);  /* sp should be set to stack top */
		} else {
			assert(cpu.get_register(i) == 0);  /* Other registers should be zero */
		}
	}

	assert(cpu.get_pc() == 0);
	assert(cpu.is_running());

	std::printf("\tOK CPU initialization works\n");
}

/* Test 2: Memory operations */
static void test_memory_operations() {
	std::printf("Test 2: Memory operations...\n");

	Memory mem(4096);
	assert(mem.get_size() == 4096);

	/* Test 8-bit read/write */
	uint8_t val8;
	assert(mem.write8(0x100, 0x42) == MEM_OK);
	assert(mem.read8(0x100, &val8) == MEM_OK);
	assert(val8 == 0x42);

	/* Test 16-bit read/write */
	uint16_t val16;
	assert(mem.write16(0x200, 0xABCD) == MEM_OK);
	assert(mem.read16(0x200, &val16) == MEM_OK);
	assert(val16 == 0xABCD);

	/* Test 32-bit read/write */
	uint32_t val32;
	assert(mem.write32(0x300, 0xDEADBEEF) == MEM_OK);
	assert(mem.read32(0x300, &val32) == MEM_OK);
	assert(val32 == 0xDEADBEEF);

	/* Test misaligned accesses */
	assert(mem.read16(0x101, &val16) == MEM_MISALIGNED_ERROR);
	assert(mem.write16(0x101, 0x1234) == MEM_MISALIGNED_ERROR);
	assert(mem.read32(0x102, &val32) == MEM_MISALIGNED_ERROR);
	assert(mem.write32(0x102, 0x12345678) == MEM_MISALIGNED_ERROR);

	/* Test out of bounds accesses */
	assert(mem.read8(0x2000, &val8) == MEM_READ_ERROR);
	assert(mem.write8(0x2000, 0x12) == MEM_WRITE_ERROR);

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

	Instruction instr;

	/* Test R-type instruction: add x1, x2, x3 */
	uint32_t add_instr = 0x003100B3;  /* add x1, x2, x3 */
	assert(instr.decode(add_instr) == true);
	assert(instr.get_format() == INSTR_R_TYPE);
	assert(instr.get_opcode() == 0x33);
	assert(instr.get_rd() == 1);
	assert(instr.get_rs1() == 2);
	assert(instr.get_rs2() == 3);
	assert(instr.get_funct3() == 0x0);
	assert(instr.get_funct7() == 0x00);

	/* Test I-type instruction: addi x1, x2, 42 */
	uint32_t addi_instr = 0x02A10093;  /* addi x1, x2, 42 */
	assert(instr.decode(addi_instr) == true);
	assert(instr.get_format() == INSTR_I_TYPE);
	assert(instr.get_opcode() == 0x13);
	assert(instr.get_rd() == 1);
	assert(instr.get_rs1() == 2);
	assert(instr.get_imm() == 42);

	/* Test S-type instruction: sw x3, 4(x2) */
	uint32_t sw_instr = 0x00312223;  /* sw x3, 4(x2) */
	assert(instr.decode(sw_instr) == true);
	assert(instr.get_format() == INSTR_S_TYPE);
	assert(instr.get_opcode() == 0x23);
	assert(instr.get_rs1() == 2);
	assert(instr.get_rs2() == 3);
	assert(instr.get_funct3() == 0x2);
	assert(instr.get_imm() == 4);

	/* Test B-type instruction: beq x2, x3, 8 */
	uint32_t beq_instr = 0x00310463;  /* beq x2, x3, 8 */
	assert(instr.decode(beq_instr) == true);
	assert(instr.get_format() == INSTR_B_TYPE);
	assert(instr.get_opcode() == 0x63);
	assert(instr.get_rs1() == 2);
	assert(instr.get_rs2() == 3);
	assert(instr.get_funct3() == 0x0);
	assert(instr.get_imm() == 8);

	/* Test U-type instruction: lui x1, 0x12345 */
	uint32_t lui_instr = 0x123450B7;  /* lui x1, 0x12345 */
	assert(instr.decode(lui_instr) == true);
	assert(instr.get_format() == INSTR_U_TYPE);
	assert(instr.get_opcode() == 0x37);
	assert(instr.get_rd() == 1);
	assert(instr.get_imm() == 0x12345000);

	/* Test J-type instruction: jal x1, 1024 */
	uint32_t jal_instr = 0x400000EF;  /* jal x1, 1024 */
	assert(instr.decode(jal_instr) == true);
	assert(instr.get_format() == INSTR_J_TYPE);
	assert(instr.get_opcode() == 0x6F);
	assert(instr.get_rd() == 1);
	assert(instr.get_imm() == 1024);

	/* Test invalid opcode */
	uint32_t invalid_instr = 0x00000000;  /* Invalid opcode */
	assert(instr.decode(invalid_instr) == false);

	std::printf("\tOK Instruction decoding works\n");
}

/* Test 5: Register read/write operations */
static void test_register_operations() {
	std::printf("Test 5: Register operations...\n");

	CPU cpu;

	/* Test that x0 is always zero */
	assert(cpu.get_register(0) == 0);  /* x0 should be zero */

	/* Test reading/writing other registers */
	for (int i = 1; i < 32; i++) {
		uint32_t value = 0x1000 + i * 0x100;
		cpu.set_register(i, value);
		assert(cpu.get_register(i) == value);
	}

	/* Verify registers maintain their values */
	assert(cpu.get_register(1) == 0x1100);
	assert(cpu.get_register(10) == 0x1A00);
	assert(cpu.get_register(31) == 0x2F00);

	std::printf("\tOK Register operations work\n");
}

/* Test 6: ALU operations */
static void test_alu_operations() {
	std::printf("Test 6: ALU operations (tested through step)...\n");

	CPU cpu;
	Memory mem(8192);

	/* Write an add instruction to memory: add x1, x2, x3 */
	uint32_t add_instr = 0x003100B3;  /* add x1, x2, x3 */
	cpu.set_pc(0x1000);
	cpu.set_register(2, 10);
	cpu.set_register(3, 20);
	mem.write32(0x1000, add_instr);

	/* Execute the instruction */
	cpu_status_t status = cpu.step(&mem);
	assert(status == CPU_OK);
	assert(cpu.get_register(1) == 30);  /* 10 + 20 = 30 */

	/* Test SUB operation: sub x4, x5, x6 */
	uint32_t sub_instr = 0x40628233;  /* sub x4, x5, x6 */
	cpu.set_pc(0x1004);
	cpu.set_register(5, 50);
	cpu.set_register(6, 30);
	mem.write32(0x1004, sub_instr);

	status = cpu.step(&mem);
	assert(status == CPU_OK);
	assert(cpu.get_register(4) == 20);  /* 50 - 30 = 20 */

	std::printf("\tOK ALU operations work (tested through step)\n");
}

/* Test 7: Load/Store operations */
static void test_load_store() {
	std::printf("Test 7: Load/Store operations...\n");

	CPU cpu;
	Memory mem(8192);

	/* Initialize some test data in memory */
	assert(mem.write32(0x200, 0x12345678) == MEM_OK);
	assert(mem.write16(0x204, 0xABCD) == MEM_OK);
	assert(mem.write8(0x206, 0x42) == MEM_OK);

	/* Test LW instruction through step */
	/* Write LW instruction: lw x1, 0x200(x2) */
	uint32_t lw_instr = 0x20012083;  /* lw x1, 0x200(x2) */
	cpu.set_pc(0x1000);
	cpu.set_register(2, 0x0);  /* Base address 0 */
	mem.write32(0x1000, lw_instr);

	cpu_status_t status = cpu.step(&mem);
	assert(status == CPU_OK);
	assert(cpu.get_register(1) == 0x12345678);  /* Should load the word */

	/* Test SW instruction through step */
	/* Write SW instruction: sw x1, 0x208(x2) */
	uint32_t sw_instr = 0x20112423;  /* sw x1, 0x208(x2) */
	cpu.set_pc(0x1004);
	cpu.set_register(1, 0xDEADBEEF);
	mem.write32(0x1004, sw_instr);

	status = cpu.step(&mem);
	assert(status == CPU_OK);

	/* Verify the store */
	uint32_t stored_word;
	assert(mem.read32(0x208, &stored_word) == MEM_OK);
	assert(stored_word == 0xDEADBEEF);

	std::printf("\tOK Load/Store operations work (tested through step)\n");
}

/* Test 8: Branch operations */
static void test_branch_operations() {
	std::printf("Test 8: Branch operations...\n");

	CPU cpu;
	Memory mem(8192);

	/* Write a BEQ instruction: beq x1, x2, 16 */
	uint32_t beq_instr = 0x00208863;  /* beq x1, x2, 16 */
	cpu.set_pc(0x1000);
	cpu.set_register(1, 42);
	cpu.set_register(2, 42);
	mem.write32(0x1000, beq_instr);

	/* Execute BEQ when registers are equal (should branch) */
	cpu_status_t status = cpu.step(&mem);
	assert(status == CPU_OK);
	assert(cpu.get_pc() == 0x1010);  /* 0x1000 + 16 */

	/* Reset and test when not equal (should not branch) */
	cpu.set_pc(0x1000);
	cpu.set_register(1, 42);
	cpu.set_register(2, 43);
	mem.write32(0x1000, beq_instr);

	status = cpu.step(&mem);
	assert(status == CPU_OK);
	assert(cpu.get_pc() == 0x1004);  /* PC should just advance by 4 */

	std::printf("\tOK Branch operations work (tested through step)\n");
}

/* Test 9: System call operations */
static void test_system_calls() {
	std::printf("Test 9: System call operations...\n");

	CPU cpu;
	Memory mem(8192);

	/* Write ECALL instruction */
	uint32_t ecall_instr = 0x00000073;  /* ecall */
	cpu.set_pc(0x1000);
	cpu.set_register(17, SYS_exit);  /* a7 = exit syscall number */
	mem.write32(0x1000, ecall_instr);

	cpu_status_t status = cpu.step(&mem);
	assert(status == CPU_SYSCALL_EXIT);
	assert(!cpu.is_running());

	std::printf("\tOK System call operations work (tested through step)\n");
}

/* Test 10: Complete CPU step execution */
static void test_cpu_step() {
	std::printf("Test 10: Complete CPU step execution...\n");

	CPU cpu;
	Memory mem(8192);

	/* Write a simple program to memory */
	/* Program: addi x1, x0, 42  (0x02A00093) */
	uint32_t program[] = {
		0x02A00093,  /* addi x1, x0, 42 */
		0x00108093,  /* addi x1, x1, 1 */
		0x00000073,  /* ecall (will exit) */
	};

	cpu.set_pc(0x1000);

	/* Write program to memory starting at PC */
	for (size_t i = 0; i < sizeof(program)/sizeof(program[0]); i++) {
		assert(mem.write32(0x1000 + i*4, program[i]) == MEM_OK);
	}

	/* Execute first instruction */
	cpu_status_t status = cpu.step(&mem);
	assert(status == CPU_OK);
	assert(cpu.get_register(1) == 42);  /* x1 should now be 42 */
	assert(cpu.get_pc() == 0x1004);  /* PC should advance by 4 */

	/* Execute second instruction */
	status = cpu.step(&mem);
	assert(status == CPU_OK);
	assert(cpu.get_register(1) == 43);  /* x1 should now be 43 */
	assert(cpu.get_pc() == 0x1008);  /* PC should advance by 4 */

	/* Setup exit syscall for third instruction */
	cpu.set_register(17, SYS_exit);  /* a7 = exit syscall number */
	cpu.set_register(10, 0);        /* a0 = exit code 0 */

	/* Execute third instruction (ecall) */
	status = cpu.step(&mem);
	assert(status == CPU_SYSCALL_EXIT);
	assert(!cpu.is_running());

	/* Test with misaligned PC */
	CPU cpu2;
	cpu2.set_pc(0x1001);  /* Misaligned PC */

	status = cpu2.step(&mem);
	assert(status == CPU_FETCH_MISALIGNED);

	/* Test with out of bounds PC */
	cpu2.set_pc(0xFFFFFFFC);  /* Near end of memory */

	status = cpu2.step(&mem);
	assert(status == CPU_FETCH_OUT_OF_BOUNDS);

	std::printf("\tOK Complete CPU step execution works\n");
}

/* Test 11: Complex instruction execution */
static void test_complex_execution() {
	std::printf("Test 11: Complex instruction execution...\n");

	CPU cpu;
	Memory mem(8192);

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

	cpu.set_pc(0x1000);

	/* Write program to memory */
	for (size_t i = 0; i < sizeof(program)/sizeof(program[0]); i++) {
		assert(mem.write32(0x1000 + i*4, program[i]) == MEM_OK);
	}

	/* Execute all instructions */
	int steps = 0;
	while (cpu.is_running() && steps < 100) {
		cpu_status_t status = cpu.step(&mem);
		if (status == CPU_SYSCALL_EXIT) break;
		assert(status == CPU_OK);
		steps++;
	}

	/* Verify results */
	assert(cpu.get_register(1) == 100);   /* x1 */
	assert(cpu.get_register(2) == 5);     /* x2 */
	assert(cpu.get_register(4) == 105);   /* x4 = x1 + x2 */
	assert(cpu.get_register(5) == 95);    /* x5 = x1 - x2 */
	assert(cpu.get_register(6) == 3200);  /* x6 = x1 << 5 */
	assert(cpu.get_register(7) == 0);     /* x7 = x1 >> 105 */

	std::printf("\tOK Complex instruction execution works\n");
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
	CPU cpu;
	assert(cpu.get_register(2) == STACK_TOP);  /* sp should be at stack top */

	std::printf("\tOK Memory layout is correct\n");
}

/* Test 13: Logical operations (AND, OR, XOR) */
static void test_logical_operations() {
	std::printf("Test 13: Logical operations...\n");

	CPU cpu;
	Memory mem(8192);

	/* Test AND: and x1, x2, x3 */
	/* x2 = 0xFF, x3 = 0x0F, result = 0x0F */
	uint32_t and_instr = 0x003170B3;  /* and x1, x2, x3 */
	cpu.set_pc(0x1000);
	cpu.set_register(2, 0xFF);
	cpu.set_register(3, 0x0F);
	mem.write32(0x1000, and_instr);

	cpu_status_t status = cpu.step(&mem);
	assert(status == CPU_OK);
	assert(cpu.get_register(1) == 0x0F);

	/* Test OR: or x4, x2, x3 */
	/* x2 = 0xF0, x3 = 0x0F, result = 0xFF */
	uint32_t or_instr = 0x00316233;  /* or x4, x2, x3 */
	cpu.set_pc(0x1004);
	cpu.set_register(2, 0xF0);
	cpu.set_register(3, 0x0F);
	mem.write32(0x1004, or_instr);

	status = cpu.step(&mem);
	assert(status == CPU_OK);
	assert(cpu.get_register(4) == 0xFF);

	/* Test XOR: xor x5, x2, x3 */
	/* x2 = 0xFF, x3 = 0x0F, result = 0xF0 */
	uint32_t xor_instr = 0x003142B3;  /* xor x5, x2, x3 */
	cpu.set_pc(0x1008);
	cpu.set_register(2, 0xFF);
	cpu.set_register(3, 0x0F);
	mem.write32(0x1008, xor_instr);

	status = cpu.step(&mem);
	assert(status == CPU_OK);
	assert(cpu.get_register(5) == 0xF0);

	/* Test ANDI: andi x6, x2, 0xF0 */
	uint32_t andi_instr = 0x0F017313;  /* andi x6, x2, 0xF0 */
	cpu.set_pc(0x100C);
	cpu.set_register(2, 0xFF);
	mem.write32(0x100C, andi_instr);

	status = cpu.step(&mem);
	assert(status == CPU_OK);
	assert(cpu.get_register(6) == 0xF0);

	/* Test ORI: ori x7, x2, 0x0F */
	uint32_t ori_instr = 0x00F16393;  /* ori x7, x2, 0x0F */
	cpu.set_pc(0x1010);
	cpu.set_register(2, 0xF0);
	mem.write32(0x1010, ori_instr);

	status = cpu.step(&mem);
	assert(status == CPU_OK);
	assert(cpu.get_register(7) == 0xFF);

	/* Test XORI: xori x8, x2, 0xFF */
	uint32_t xori_instr = 0x0FF14413;  /* xori x8, x2, 0xFF */
	cpu.set_pc(0x1014);
	cpu.set_register(2, 0xAA);
	mem.write32(0x1014, xori_instr);

	status = cpu.step(&mem);
	assert(status == CPU_OK);
	assert(cpu.get_register(8) == 0x55);  /* 0xAA ^ 0xFF = 0x55 */

	std::printf("\tOK Logical operations work\n");
}

/* Test 14: Shift operations */
static void test_shift_operations() {
	std::printf("Test 14: Shift operations...\n");

	CPU cpu;
	Memory mem(8192);

	/* Test SLL: sll x1, x2, x3 */
	/* x2 = 0x10, x3 = 2, result = 0x40 */
	uint32_t sll_instr = 0x003110B3;  /* sll x1, x2, x3 */
	cpu.set_pc(0x1000);
	cpu.set_register(2, 0x10);
	cpu.set_register(3, 2);
	mem.write32(0x1000, sll_instr);

	cpu_status_t status = cpu.step(&mem);
	assert(status == CPU_OK);
	assert(cpu.get_register(1) == 0x40);

	/* Test SRL: srl x4, x2, x3 */
	/* x2 = 0x80, x3 = 2, result = 0x20 */
	uint32_t srl_instr = 0x00315233;  /* srl x4, x2, x3 */
	cpu.set_pc(0x1004);
	cpu.set_register(2, 0x80);
	cpu.set_register(3, 2);
	mem.write32(0x1004, srl_instr);

	status = cpu.step(&mem);
	assert(status == CPU_OK);
	assert(cpu.get_register(4) == 0x20);

	/* Test SRA: sra x5, x2, x3 */
	/* x2 = 0x80000000 (negative), x3 = 1, result = 0xC0000000 (sign extended) */
	uint32_t sra_instr = 0x403152B3;  /* sra x5, x2, x3 */
	cpu.set_pc(0x1008);
	cpu.set_register(2, 0x80000000);
	cpu.set_register(3, 1);
	mem.write32(0x1008, sra_instr);

	status = cpu.step(&mem);
	assert(status == CPU_OK);
	assert(cpu.get_register(5) == 0xC0000000);

	/* Test SLLI: slli x6, x2, 3 */
	uint32_t slli_instr = 0x00311313;  /* slli x6, x2, 3 */
	cpu.set_pc(0x100C);
	cpu.set_register(2, 0x10);
	mem.write32(0x100C, slli_instr);

	status = cpu.step(&mem);
	assert(status == CPU_OK);
	assert(cpu.get_register(6) == 0x80);

	/* Test SRLI: srli x7, x2, 4 */
	uint32_t srli_instr = 0x00415393;  /* srli x7, x2, 4 */
	cpu.set_pc(0x1010);
	cpu.set_register(2, 0xF0);
	mem.write32(0x1010, srli_instr);

	status = cpu.step(&mem);
	assert(status == CPU_OK);
	assert(cpu.get_register(7) == 0x0F);

	/* Test SRAI: srai x8, x2, 2 */
	/* x2 = 0x80000000, shift right 2 with sign extension = 0xE0000000 */
	uint32_t srai_instr = 0x40215413;  /* srai x8, x2, 2 */
	cpu.set_pc(0x1014);
	cpu.set_register(2, 0x80000000);
	mem.write32(0x1014, srai_instr);

	status = cpu.step(&mem);
	assert(status == CPU_OK);
	assert(cpu.get_register(8) == 0xE0000000);

	std::printf("\tOK Shift operations work\n");
}

/* Test 15: Comparison operations */
static void test_comparison_operations() {
	std::printf("Test 15: Comparison operations...\n");

	CPU cpu;
	Memory mem(8192);

	/* Test SLT: slt x1, x2, x3 (signed comparison) */
	/* x2 = -1 (0xFFFFFFFF), x3 = 1, result = 1 (true, -1 < 1) */
	uint32_t slt_instr = 0x003120B3;  /* slt x1, x2, x3 */
	cpu.set_pc(0x1000);
	cpu.set_register(2, 0xFFFFFFFF);  /* -1 in two's complement */
	cpu.set_register(3, 1);
	mem.write32(0x1000, slt_instr);

	cpu_status_t status = cpu.step(&mem);
	assert(status == CPU_OK);
	assert(cpu.get_register(1) == 1);

	/* Test SLTU: sltu x4, x2, x3 (unsigned comparison) */
	/* x2 = 0xFFFFFFFF, x3 = 1, result = 0 (false, 0xFFFFFFFF > 1 unsigned) */
	uint32_t sltu_instr = 0x00313233;  /* sltu x4, x2, x3 */
	cpu.set_pc(0x1004);
	cpu.set_register(2, 0xFFFFFFFF);
	cpu.set_register(3, 1);
	mem.write32(0x1004, sltu_instr);

	status = cpu.step(&mem);
	assert(status == CPU_OK);
	assert(cpu.get_register(4) == 0);

	/* Test SLTI: slti x5, x2, 10 */
	/* x2 = 5, immediate = 10, result = 1 (true, 5 < 10) */
	uint32_t slti_instr = 0x00A12293;  /* slti x5, x2, 10 */
	cpu.set_pc(0x1008);
	cpu.set_register(2, 5);
	mem.write32(0x1008, slti_instr);

	status = cpu.step(&mem);
	assert(status == CPU_OK);
	assert(cpu.get_register(5) == 1);

	/* Test SLTIU: sltiu x6, x2, 10 */
	/* x2 = 15, immediate = 10, result = 0 (false, 15 > 10) */
	uint32_t sltiu_instr = 0x00A13313;  /* sltiu x6, x2, 10 */
	cpu.set_pc(0x100C);
	cpu.set_register(2, 15);
	mem.write32(0x100C, sltiu_instr);

	status = cpu.step(&mem);
	assert(status == CPU_OK);
	assert(cpu.get_register(6) == 0);

	std::printf("\tOK Comparison operations work\n");
}

/* Test 16: All load variants (lb, lh, lbu, lhu) */
static void test_load_variants() {
	std::printf("Test 16: Load variants...\n");

	CPU cpu;
	Memory mem(8192);

	/* Setup test data in memory */
	/* Address 0x200: 0x89ABCDEF (little-endian: EF CD AB 89) */
	mem.write8(0x200, 0xEF);
	mem.write8(0x201, 0xCD);
	mem.write8(0x202, 0xAB);
	mem.write8(0x203, 0x89);

	/* Test LB: lb x1, 0x200(x0) - load signed byte */
	/* Byte at 0x200 = 0xEF, sign-extended to 0xFFFFFFEF */
	uint32_t lb_instr = 0x20000083;  /* lb x1, 0x200(x0) */
	cpu.set_pc(0x1000);
	mem.write32(0x1000, lb_instr);

	cpu_status_t status = cpu.step(&mem);
	assert(status == CPU_OK);
	assert(cpu.get_register(1) == 0xFFFFFFEF);

	/* Test LBU: lbu x2, 0x200(x0) - load unsigned byte */
	/* Byte at 0x200 = 0xEF, zero-extended to 0x000000EF */
	uint32_t lbu_instr = 0x20004103;  /* lbu x2, 0x200(x0) */
	cpu.set_pc(0x1004);
	mem.write32(0x1004, lbu_instr);

	status = cpu.step(&mem);
	assert(status == CPU_OK);
	assert(cpu.get_register(2) == 0x000000EF);

	/* Test LH: lh x3, 0x200(x0) - load signed halfword */
	/* Halfword at 0x200 = 0xCDEF, sign-extended to 0xFFFFCDEF */
	uint32_t lh_instr = 0x20001183;  /* lh x3, 0x200(x0) */
	cpu.set_pc(0x1008);
	mem.write32(0x1008, lh_instr);

	status = cpu.step(&mem);
	assert(status == CPU_OK);
	assert(cpu.get_register(3) == 0xFFFFCDEF);

	/* Test LHU: lhu x4, 0x200(x0) - load unsigned halfword */
	/* Halfword at 0x200 = 0xCDEF, zero-extended to 0x0000CDEF */
	uint32_t lhu_instr = 0x20005203;  /* lhu x4, 0x200(x0) */
	cpu.set_pc(0x100C);
	mem.write32(0x100C, lhu_instr);

	status = cpu.step(&mem);
	assert(status == CPU_OK);
	assert(cpu.get_register(4) == 0x0000CDEF);

	std::printf("\tOK Load variants work\n");
}

/* Test 17: All store variants (sb, sh, sw) */
static void test_store_variants() {
	std::printf("Test 17: Store variants...\n");

	CPU cpu;
	Memory mem(8192);

	/* Test SB: sb x1, 0x300(x0) - store byte */
	uint32_t sb_instr = 0x30100023;  /* sb x1, 0x300(x0) */
	cpu.set_pc(0x1000);
	cpu.set_register(1, 0x12345678);  /* Only 0x78 should be stored */
	mem.write32(0x1000, sb_instr);

	cpu_status_t status = cpu.step(&mem);
	assert(status == CPU_OK);

	uint8_t val8;
	mem.read8(0x300, &val8);
	assert(val8 == 0x78);

	/* Test SH: sh x2, 0x302(x0) - store halfword */
	uint32_t sh_instr = 0x30201123;  /* sh x2, 0x302(x0) */
	cpu.set_pc(0x1004);
	cpu.set_register(2, 0xABCD1234);  /* Only 0x1234 should be stored */
	mem.write32(0x1004, sh_instr);

	status = cpu.step(&mem);
	assert(status == CPU_OK);

	uint16_t val16;
	mem.read16(0x302, &val16);
	assert(val16 == 0x1234);

	/* Test SW: sw x3, 0x304(x0) - store word */
	uint32_t sw_instr = 0x30302223;  /* sw x3, 0x304(x0) */
	cpu.set_pc(0x1008);
	cpu.set_register(3, 0xDEADBEEF);
	mem.write32(0x1008, sw_instr);

	status = cpu.step(&mem);
	assert(status == CPU_OK);

	uint32_t val32;
	mem.read32(0x304, &val32);
	assert(val32 == 0xDEADBEEF);

	std::printf("\tOK Store variants work\n");
}

/* Test 18: All branch variants */
static void test_all_branch_variants() {
	std::printf("Test 18: All branch variants...\n");

	CPU cpu;
	Memory mem(8192);

	/* Test BNE: bne x1, x2, 16 (branch if not equal) */
	uint32_t bne_instr = 0x00209863;  /* bne x1, x2, 16 */
	cpu.set_pc(0x1000);
	cpu.set_register(1, 10);
	cpu.set_register(2, 20);
	mem.write32(0x1000, bne_instr);

	cpu_status_t status = cpu.step(&mem);
	assert(status == CPU_OK);
	assert(cpu.get_pc() == 0x1010);  /* Should branch */

	/* Test BLT: blt x1, x2, 16 (branch if less than, signed) */
	uint32_t blt_instr = 0x0020C863;  /* blt x1, x2, 16 */
	cpu.set_pc(0x1000);
	cpu.set_register(1, 10);
	cpu.set_register(2, 20);
	mem.write32(0x1000, blt_instr);

	status = cpu.step(&mem);
	assert(status == CPU_OK);
	assert(cpu.get_pc() == 0x1010);  /* Should branch (10 < 20) */

	/* Test BGE: bge x1, x2, 16 (branch if greater or equal, signed) */
	uint32_t bge_instr = 0x0020D863;  /* bge x1, x2, 16 */
	cpu.set_pc(0x1000);
	cpu.set_register(1, 20);
	cpu.set_register(2, 10);
	mem.write32(0x1000, bge_instr);

	status = cpu.step(&mem);
	assert(status == CPU_OK);
	assert(cpu.get_pc() == 0x1010);  /* Should branch (20 >= 10) */

	/* Test BLTU: bltu x1, x2, 16 (branch if less than, unsigned) */
	uint32_t bltu_instr = 0x0020E863;  /* bltu x1, x2, 16 */
	cpu.set_pc(0x1000);
	cpu.set_register(1, 5);
	cpu.set_register(2, 10);
	mem.write32(0x1000, bltu_instr);

	status = cpu.step(&mem);
	assert(status == CPU_OK);
	assert(cpu.get_pc() == 0x1010);  /* Should branch (5 < 10 unsigned) */

	/* Test BGEU: bgeu x1, x2, 16 (branch if greater or equal, unsigned) */
	uint32_t bgeu_instr = 0x0020F863;  /* bgeu x1, x2, 16 */
	cpu.set_pc(0x1000);
	cpu.set_register(1, 0xFFFFFFFF);  /* Large unsigned value */
	cpu.set_register(2, 10);
	mem.write32(0x1000, bgeu_instr);

	status = cpu.step(&mem);
	assert(status == CPU_OK);
	assert(cpu.get_pc() == 0x1010);  /* Should branch (0xFFFFFFFF >= 10 unsigned) */

	std::printf("\tOK All branch variants work\n");
}

/* Test 19: JALR instruction */
static void test_jalr_instruction() {
	std::printf("Test 19: JALR instruction...\n");

	CPU cpu;
	Memory mem(8192);

	/* Test JALR: jalr x1, x2, 8 */
	/* Jump to address in x2 + 8, store return address in x1 */
	uint32_t jalr_instr = 0x008100E7;  /* jalr x1, x2, 8 */
	cpu.set_pc(0x1000);
	cpu.set_register(2, 0x2000);
	mem.write32(0x1000, jalr_instr);

	cpu_status_t status = cpu.step(&mem);
	assert(status == CPU_OK);
	assert(cpu.get_pc() == 0x2008);  /* 0x2000 + 8 */
	assert(cpu.get_register(1) == 0x1004);  /* Return address */

	std::printf("\tOK JALR instruction works\n");
}

/* Test 20: AUIPC instruction */
static void test_auipc_instruction() {
	std::printf("Test 20: AUIPC instruction...\n");

	CPU cpu;
	Memory mem(8192);

	/* Test AUIPC: auipc x1, 0x12345 */
	/* Add upper immediate to PC: x1 = PC + (0x12345 << 12) */
	uint32_t auipc_instr = 0x12345097;  /* auipc x1, 0x12345 */
	cpu.set_pc(0x1000);
	mem.write32(0x1000, auipc_instr);

	cpu_status_t status = cpu.step(&mem);
	assert(status == CPU_OK);
	assert(cpu.get_register(1) == 0x1000 + 0x12345000);  /* PC + upper immediate */

	std::printf("\tOK AUIPC instruction works\n");
}

/* Test 21: M Extension - MUL instruction */
static void test_m_extension_mul() {
	std::printf("Test 21: M Extension - MUL instruction...\n");

	CPU cpu;
	Memory mem(8192);

	/* Test basic multiplication: mul x1, x2, x3 */
	/* Encoding: funct7=0x01, rs2, rs1, funct3=0x0, rd, opcode=0x33 */
	uint32_t mul_instr = 0x023100B3;  /* mul x1, x2, x3 */
	cpu.set_pc(0x1000);
	cpu.set_register(2, 6);
	cpu.set_register(3, 7);
	mem.write32(0x1000, mul_instr);

	cpu_status_t status = cpu.step(&mem);
	assert(status == CPU_OK);
	assert(cpu.get_register(1) == 42);  /* 6 * 7 = 42 */

	/* Test multiplication with negative numbers */
	cpu.set_pc(0x1004);
	cpu.set_register(2, (uint32_t)-5);  /* -5 in 32-bit two's complement */
	cpu.set_register(3, 10);
	mem.write32(0x1004, mul_instr);

	status = cpu.step(&mem);
	assert(status == CPU_OK);
	assert(cpu.get_register(1) == (uint32_t)-50);  /* -5 * 10 = -50 */

	/* Test multiplication with large numbers */
	cpu.set_pc(0x1008);
	cpu.set_register(2, 0x12345678);
	cpu.set_register(3, 2);
	mem.write32(0x1008, mul_instr);

	status = cpu.step(&mem);
	assert(status == CPU_OK);
	assert(cpu.get_register(1) == 0x2468ACF0);  /* Lower 32 bits of product */

	/* Test multiplication with zero */
	cpu.set_pc(0x100C);
	cpu.set_register(2, 12345);
	cpu.set_register(3, 0);
	mem.write32(0x100C, mul_instr);

	status = cpu.step(&mem);
	assert(status == CPU_OK);
	assert(cpu.get_register(1) == 0);  /* Any number * 0 = 0 */

	std::printf("\tOK MUL instruction works\n");
}

/* Test 22: M Extension - MULH instruction */
static void test_m_extension_mulh() {
	std::printf("Test 22: M Extension - MULH instruction...\n");

	CPU cpu;
	Memory mem(8192);
	cpu_status_t status;

	/* Test MULH: mulh x1, x2, x3 */
	/* Returns upper 32 bits of signed multiplication */
	/* Encoding: funct7=0x01, rs2, rs1, funct3=0x1, rd, opcode=0x33 */
	uint32_t mulh_instr = 0x023110B3;  /* mulh x1, x2, x3 */

	/* Test with positive numbers that overflow 32 bits */
	cpu.set_pc(0x1000);
	cpu.set_register(2, 0x80000000);  /* 2^31 (large positive when unsigned) */
	cpu.set_register(3, 2);
	mem.write32(0x1000, mulh_instr);

	status = cpu.step(&mem);
	assert(status == CPU_OK);
	/* -2147483648 * 2 = -4294967296, upper 32 bits = -1 (0xFFFFFFFF) */
	assert(cpu.get_register(1) == 0xFFFFFFFF);

	/* Test with positive numbers */
	cpu.set_pc(0x1004);
	cpu.set_register(2, 0x40000000);  /* 2^30 */
	cpu.set_register(3, 4);
	mem.write32(0x1004, mulh_instr);

	status = cpu.step(&mem);
	assert(status == CPU_OK);
	assert(cpu.get_register(1) == 1);  /* 2^30 * 4 = 2^32, upper 32 bits = 1 */

	/* Test negative * negative */
	cpu.set_pc(0x1008);
	cpu.set_register(2, (uint32_t)-2);
	cpu.set_register(3, (uint32_t)-3);
	mem.write32(0x1008, mulh_instr);

	status = cpu.step(&mem);
	assert(status == CPU_OK);
	assert(cpu.get_register(1) == 0);  /* -2 * -3 = 6, upper 32 bits = 0 */

	std::printf("\tOK MULH instruction works\n");
}

/* Test 23: M Extension - MULHSU instruction */
static void test_m_extension_mulhsu() {
	std::printf("Test 23: M Extension - MULHSU instruction...\n");

	CPU cpu;
	Memory mem(8192);
	cpu_status_t status;

	/* Test MULHSU: mulhsu x1, x2, x3 */
	/* Returns upper 32 bits of signed rs1 * unsigned rs2 */
	/* Encoding: funct7=0x01, rs2, rs1, funct3=0x2, rd, opcode=0x33 */
	uint32_t mulhsu_instr = 0x023120B3;  /* mulhsu x1, x2, x3 */

	/* Test signed * unsigned */
	cpu.set_pc(0x1000);
	cpu.set_register(2, (uint32_t)-2);  /* -2 signed */
	cpu.set_register(3, 0x80000000);     /* Large unsigned */
	mem.write32(0x1000, mulhsu_instr);

	status = cpu.step(&mem);
	assert(status == CPU_OK);
	assert(cpu.get_register(1) == 0xFFFFFFFF);  /* Negative result */

	/* Test positive signed * unsigned */
	cpu.set_pc(0x1004);
	cpu.set_register(2, 2);
	cpu.set_register(3, 0x80000000);
	mem.write32(0x1004, mulhsu_instr);

	status = cpu.step(&mem);
	assert(status == CPU_OK);
	assert(cpu.get_register(1) == 1);  /* Upper 32 bits */

	std::printf("\tOK MULHSU instruction works\n");
}

/* Test 24: M Extension - MULHU instruction */
static void test_m_extension_mulhu() {
	std::printf("Test 24: M Extension - MULHU instruction...\n");

	CPU cpu;
	Memory mem(8192);
	cpu_status_t status;

	/* Test MULHU: mulhu x1, x2, x3 */
	/* Returns upper 32 bits of unsigned multiplication */
	/* Encoding: funct7=0x01, rs2, rs1, funct3=0x3, rd, opcode=0x33 */
	uint32_t mulhu_instr = 0x023130B3;  /* mulhu x1, x2, x3 */

	/* Test large unsigned multiplication */
	cpu.set_pc(0x1000);
	cpu.set_register(2, 0xFFFFFFFF);  /* Max unsigned 32-bit */
	cpu.set_register(3, 0xFFFFFFFF);  /* Max unsigned 32-bit */
	mem.write32(0x1000, mulhu_instr);

	status = cpu.step(&mem);
	assert(status == CPU_OK);
	assert(cpu.get_register(1) == 0xFFFFFFFE);  /* Upper 32 bits */

	/* Test with smaller numbers */
	cpu.set_pc(0x1004);
	cpu.set_register(2, 0x80000000);
	cpu.set_register(3, 2);
	mem.write32(0x1004, mulhu_instr);

	status = cpu.step(&mem);
	assert(status == CPU_OK);
	assert(cpu.get_register(1) == 1);  /* Upper 32 bits */

	/* Test with zero */
	cpu.set_pc(0x1008);
	cpu.set_register(2, 0xFFFFFFFF);
	cpu.set_register(3, 0);
	mem.write32(0x1008, mulhu_instr);

	status = cpu.step(&mem);
	assert(status == CPU_OK);
	assert(cpu.get_register(1) == 0);  /* 0 * anything = 0 */

	std::printf("\tOK MULHU instruction works\n");
}

/* Test 25: M Extension - DIV instruction */
static void test_m_extension_div() {
	std::printf("Test 25: M Extension - DIV instruction...\n");

	CPU cpu;
	Memory mem(8192);

	/* Test DIV: div x1, x2, x3 */
	/* Signed division */
	/* Encoding: funct7=0x01, rs2, rs1, funct3=0x4, rd, opcode=0x33 */
	uint32_t div_instr = 0x023140B3;  /* div x1, x2, x3 */

	/* Test basic division */
	cpu.set_pc(0x1000);
	cpu.set_register(2, 42);
	cpu.set_register(3, 7);
	mem.write32(0x1000, div_instr);

	cpu_status_t status = cpu.step(&mem);
	assert(status == CPU_OK);
	assert(cpu.get_register(1) == 6);  /* 42 / 7 = 6 */

	/* Test division with negative dividend */
	cpu.set_pc(0x1004);
	cpu.set_register(2, (uint32_t)-20);
	cpu.set_register(3, 4);
	mem.write32(0x1004, div_instr);

	status = cpu.step(&mem);
	assert(status == CPU_OK);
	assert(cpu.get_register(1) == (uint32_t)-5);  /* -20 / 4 = -5 */

	/* Test division with negative divisor */
	cpu.set_pc(0x1008);
	cpu.set_register(2, 20);
	cpu.set_register(3, (uint32_t)-4);
	mem.write32(0x1008, div_instr);

	status = cpu.step(&mem);
	assert(status == CPU_OK);
	assert(cpu.get_register(1) == (uint32_t)-5);  /* 20 / -4 = -5 */

	/* Test division by zero (should return -1) */
	cpu.set_pc(0x100C);
	cpu.set_register(2, 100);
	cpu.set_register(3, 0);
	mem.write32(0x100C, div_instr);

	status = cpu.step(&mem);
	assert(status == CPU_OK);
	assert(cpu.get_register(1) == 0xFFFFFFFF);  /* Division by zero returns -1 */

	/* Test rounding towards zero */
	cpu.set_pc(0x1010);
	cpu.set_register(2, 7);
	cpu.set_register(3, 2);
	mem.write32(0x1010, div_instr);

	status = cpu.step(&mem);
	assert(status == CPU_OK);
	assert(cpu.get_register(1) == 3);  /* 7 / 2 = 3 (rounds towards zero) */

	std::printf("\tOK DIV instruction works\n");
}

/* Test 26: M Extension - DIVU instruction */
static void test_m_extension_divu() {
	std::printf("Test 26: M Extension - DIVU instruction...\n");

	CPU cpu;
	Memory mem(8192);

	/* Test DIVU: divu x1, x2, x3 */
	/* Unsigned division */
	/* Encoding: funct7=0x01, rs2, rs1, funct3=0x5, rd, opcode=0x33 */
	uint32_t divu_instr = 0x023150B3;  /* divu x1, x2, x3 */

	/* Test basic unsigned division */
	cpu.set_pc(0x1000);
	cpu.set_register(2, 100);
	cpu.set_register(3, 3);
	mem.write32(0x1000, divu_instr);

	cpu_status_t status = cpu.step(&mem);
	assert(status == CPU_OK);
	assert(cpu.get_register(1) == 33);  /* 100 / 3 = 33 */

	/* Test with large unsigned numbers */
	cpu.set_pc(0x1004);
	cpu.set_register(2, 0xFFFFFFFF);  /* Max unsigned */
	cpu.set_register(3, 2);
	mem.write32(0x1004, divu_instr);

	status = cpu.step(&mem);
	assert(status == CPU_OK);
	assert(cpu.get_register(1) == 0x7FFFFFFF);  /* 4294967295 / 2 = 2147483647 */

	/* Test division by zero (should return 2^32-1) */
	cpu.set_pc(0x1008);
	cpu.set_register(2, 100);
	cpu.set_register(3, 0);
	mem.write32(0x1008, divu_instr);

	status = cpu.step(&mem);
	assert(status == CPU_OK);
	assert(cpu.get_register(1) == 0xFFFFFFFF);  /* Division by zero returns max value */

	std::printf("\tOK DIVU instruction works\n");
}

/* Test 27: M Extension - REM instruction */
static void test_m_extension_rem() {
	std::printf("Test 27: M Extension - REM instruction...\n");

	CPU cpu;
	Memory mem(8192);

	/* Test REM: rem x1, x2, x3 */
	/* Signed remainder */
	/* Encoding: funct7=0x01, rs2, rs1, funct3=0x6, rd, opcode=0x33 */
	uint32_t rem_instr = 0x023160B3;  /* rem x1, x2, x3 */

	/* Test basic remainder */
	cpu.set_pc(0x1000);
	cpu.set_register(2, 42);
	cpu.set_register(3, 7);
	mem.write32(0x1000, rem_instr);

	cpu_status_t status = cpu.step(&mem);
	assert(status == CPU_OK);
	assert(cpu.get_register(1) == 0);  /* 42 % 7 = 0 */

	/* Test remainder with non-zero result */
	cpu.set_pc(0x1004);
	cpu.set_register(2, 43);
	cpu.set_register(3, 7);
	mem.write32(0x1004, rem_instr);

	status = cpu.step(&mem);
	assert(status == CPU_OK);
	assert(cpu.get_register(1) == 1);  /* 43 % 7 = 1 */

	/* Test remainder with negative dividend */
	cpu.set_pc(0x1008);
	cpu.set_register(2, (uint32_t)-23);
	cpu.set_register(3, 5);
	mem.write32(0x1008, rem_instr);

	status = cpu.step(&mem);
	assert(status == CPU_OK);
	assert(cpu.get_register(1) == (uint32_t)-3);  /* -23 % 5 = -3 */

	/* Test remainder with negative divisor */
	cpu.set_pc(0x100C);
	cpu.set_register(2, 23);
	cpu.set_register(3, (uint32_t)-5);
	mem.write32(0x100C, rem_instr);

	status = cpu.step(&mem);
	assert(status == CPU_OK);
	assert(cpu.get_register(1) == 3);  /* 23 % -5 = 3 */

	/* Test remainder by zero (should return dividend) */
	cpu.set_pc(0x1010);
	cpu.set_register(2, 100);
	cpu.set_register(3, 0);
	mem.write32(0x1010, rem_instr);

	status = cpu.step(&mem);
	assert(status == CPU_OK);
	assert(cpu.get_register(1) == 100);  /* Remainder by zero returns dividend */

	std::printf("\tOK REM instruction works\n");
}

/* Test 28: M Extension - REMU instruction */
static void test_m_extension_remu() {
	std::printf("Test 28: M Extension - REMU instruction...\n");

	CPU cpu;
	Memory mem(8192);

	/* Test REMU: remu x1, x2, x3 */
	/* Unsigned remainder */
	/* Encoding: funct7=0x01, rs2, rs1, funct3=0x7, rd, opcode=0x33 */
	uint32_t remu_instr = 0x023170B3;  /* remu x1, x2, x3 */

	/* Test basic unsigned remainder */
	cpu.set_pc(0x1000);
	cpu.set_register(2, 100);
	cpu.set_register(3, 3);
	mem.write32(0x1000, remu_instr);

	cpu_status_t status = cpu.step(&mem);
	assert(status == CPU_OK);
	assert(cpu.get_register(1) == 1);  /* 100 % 3 = 1 */

	/* Test with large unsigned numbers */
	cpu.set_pc(0x1004);
	cpu.set_register(2, 0xFFFFFFFF);
	cpu.set_register(3, 10);
	mem.write32(0x1004, remu_instr);

	status = cpu.step(&mem);
	assert(status == CPU_OK);
	assert(cpu.get_register(1) == 5);  /* 4294967295 % 10 = 5 */

	/* Test remainder by zero (should return dividend) */
	cpu.set_pc(0x1008);
	cpu.set_register(2, 42);
	cpu.set_register(3, 0);
	mem.write32(0x1008, remu_instr);

	status = cpu.step(&mem);
	assert(status == CPU_OK);
	assert(cpu.get_register(1) == 42);  /* Remainder by zero returns dividend */

	/* Test zero remainder */
	cpu.set_pc(0x100C);
	cpu.set_register(2, 50);
	cpu.set_register(3, 5);
	mem.write32(0x100C, remu_instr);

	status = cpu.step(&mem);
	assert(status == CPU_OK);
	assert(cpu.get_register(1) == 0);  /* 50 % 5 = 0 */

	std::printf("\tOK REMU instruction works\n");
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
	test_logical_operations(); test_count++;
	test_shift_operations(); test_count++;
	test_comparison_operations(); test_count++;
	test_load_variants(); test_count++;
	test_store_variants(); test_count++;
	test_all_branch_variants(); test_count++;
	test_jalr_instruction(); test_count++;
	test_auipc_instruction(); test_count++;

	/* M Extension tests */
	test_m_extension_mul(); test_count++;
	test_m_extension_mulh(); test_count++;
	test_m_extension_mulhsu(); test_count++;
	test_m_extension_mulhu(); test_count++;
	test_m_extension_div(); test_count++;
	test_m_extension_divu(); test_count++;
	test_m_extension_rem(); test_count++;
	test_m_extension_remu(); test_count++;

	std::printf("\n=== All %d tests passed! ===\n", test_count);
	return 0;
}
