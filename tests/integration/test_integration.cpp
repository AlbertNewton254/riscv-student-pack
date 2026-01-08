/* test_integration.cpp */
#include "../../assembler/include/assembler.hpp"
#include "../../emulator/include/cpu.hpp"
#include "../../emulator/include/memory.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <memory>

/* Helper function to assemble code from string to memory buffer */
static bool assemble_to_memory(const char *asm_code, uint8_t *buffer, size_t buffer_size, uint32_t *bytes_written) {
	/* Create temporary files for input and output */
	FILE *in = tmpfile();
	FILE *out = tmpfile();

	if (!in || !out) {
		if (in) fclose(in);
		if (out) fclose(out);
		return false;
	}

	/* Write assembly code to input file */
	fwrite(asm_code, 1, strlen(asm_code), in);
	rewind(in);

	/* Assemble the code */
	Assembler assembler;
	assembler.set_debug_mode(false);

	assembler.first_pass(in);
	assembler.adjust_labels(assembler.get_text_size());
	assembler.second_pass(in, out);

	/* Read assembled binary */
	rewind(out);
	*bytes_written = fread(buffer, 1, buffer_size, out);

	fclose(in);
	fclose(out);

	return *bytes_written > 0;
}

/* Test 1: Simple arithmetic program */
static void test_simple_arithmetic() {
	std::printf("Test 1: Simple arithmetic program...\n");

	const char *asm_code =
		".text\n"
		"main:\n"
		"    li a0, 10\n"
		"    li a1, 20\n"
		"    add a2, a0, a1\n"
		"    li a7, 93\n"
		"    ecall\n";

	uint8_t binary[1024];
	uint32_t size;

	assert(assemble_to_memory(asm_code, binary, sizeof(binary), &size));
	assert(size > 0);

	/* Create memory and CPU */
	auto mem = std::make_unique<Memory>(MEMORY_SIZE);
	auto cpu = std::make_unique<CPU>();

	/* Load program into memory */
	memcpy(&mem->get_data()[0], binary, size);
	cpu->set_pc(0);

	/* Execute until exit */
	int step_count = 0;
	while (cpu->is_running() && step_count < 1000) {
		cpu_status_t status = cpu->step(mem.get());
		step_count++;

		if (status == CPU_SYSCALL_EXIT) {
			break;
		}
		assert(status == CPU_OK);
	}

	/* Debug: check register values */
	uint32_t a0 = cpu->get_register(10);
	uint32_t a2 = cpu->get_register(12);

	/* Verify result: a2 should contain 30 */
	if (a2 != 30) {
		std::printf("\tERROR: Expected a2=30, got a2=%u\n", a2);
	}
	assert(a2 == 30);  /* a2 = x12 */

	/* Note: a0 is the exit code, which might be the sum we calculated */
	std::printf("\tOK Simple arithmetic works (10 + 20 = 30, a0=%u, a2=%u)\n", a0, a2);
}

/* Test 2: Loop and accumulator */
static void test_loop_accumulator() {
	std::printf("Test 2: Loop and accumulator...\n");

	const char *asm_code =
		".text\n"
		"main:\n"
		"    li a0, 0      # sum = 0\n"
		"    li a1, 1      # i = 1\n"
		"    li a2, 11     # limit = 11 (exclusive)\n"
		"loop:\n"
		"    add a0, a0, a1  # sum += i\n"
		"    addi a1, a1, 1  # i++\n"
		"    blt a1, a2, loop # if i < 11, continue\n"
		"    li a7, 93\n"
		"    ecall\n";

	uint8_t binary[1024];
	uint32_t size;

	assert(assemble_to_memory(asm_code, binary, sizeof(binary), &size));

	auto mem = std::make_unique<Memory>(MEMORY_SIZE);
	auto cpu = std::make_unique<CPU>();

	memcpy(&mem->get_data()[0], binary, size);
	cpu->set_pc(0);

	int step_count = 0;
	while (cpu->is_running() && step_count < 10000) {
		cpu_status_t status = cpu->step(mem.get());
		step_count++;

		if (status == CPU_SYSCALL_EXIT) {
			break;
		}
		assert(status == CPU_OK);
	}

	/* Verify result: sum of 1 to 10 = 55 */
	assert(cpu->get_register(10) == 55);  /* a0 = x10 */

	std::printf("\tOK Loop accumulator works (sum 1-10 = 55)\n");
}

/* Test 3: Memory load/store operations */
static void test_memory_operations() {
	std::printf("Test 3: Memory load/store operations...\n");

	const char *asm_code =
		".text\n"
		"main:\n"
		"    li a0, 0x1000   # address\n"
		"    li a1, 42       # value to store\n"
		"    sw a1, 0(a0)    # store word\n"
		"    lw a2, 0(a0)    # load word back\n"
		"    li a0, 0\n"
		"    add a0, a0, a2  # move result to a0 for exit\n"
		"    li a7, 93\n"
		"    ecall\n";

	uint8_t binary[1024];
	uint32_t size;

	assert(assemble_to_memory(asm_code, binary, sizeof(binary), &size));

	auto mem = std::make_unique<Memory>(MEMORY_SIZE);
	auto cpu = std::make_unique<CPU>();

	memcpy(&mem->get_data()[0], binary, size);
	cpu->set_pc(0);

	int step_count = 0;
	while (cpu->is_running() && step_count < 1000) {
		cpu_status_t status = cpu->step(mem.get());
		step_count++;

		if (status == CPU_SYSCALL_EXIT) {
			break;
		}
		assert(status == CPU_OK);
	}

	/* Verify result: should have loaded 42 */
	assert(cpu->get_register(10) == 42);  /* a0 = x10 */

	std::printf("\tOK Memory operations work (stored and loaded 42)\n");
}

/* Test 4: Function call and return */
static void test_function_call() {
	std::printf("Test 4: Function call and return...\n");

	const char *asm_code =
		".text\n"
		"main:\n"
		"    li a0, 5\n"
		"    li a1, 7\n"
		"    call multiply\n"
		"    li a7, 93\n"
		"    ecall\n"
		"\n"
		"multiply:\n"
		"    li a2, 0       # result = 0\n"
		"    li t0, 0       # i = 0\n"
		"mult_loop:\n"
		"    beq t0, a0, mult_done\n"
		"    add a2, a2, a1 # result += a1\n"
		"    addi t0, t0, 1 # i++\n"
		"    j mult_loop\n"
		"mult_done:\n"
		"    mv a0, a2      # return result in a0\n"
		"    ret\n";

	uint8_t binary[1024];
	uint32_t size;

	assert(assemble_to_memory(asm_code, binary, sizeof(binary), &size));

	auto mem = std::make_unique<Memory>(MEMORY_SIZE);
	auto cpu = std::make_unique<CPU>();

	memcpy(&mem->get_data()[0], binary, size);
	cpu->set_pc(0);

	int step_count = 0;
	while (cpu->is_running() && step_count < 10000) {
		cpu_status_t status = cpu->step(mem.get());
		step_count++;

		if (status == CPU_SYSCALL_EXIT) {
			break;
		}
		assert(status == CPU_OK);
	}

	/* Verify result: 5 * 7 = 35 */
	assert(cpu->get_register(10) == 35);  /* a0 = x10 */

	std::printf("\tOK Function call works (5 * 7 = 35)\n");
}

/* Test 5: Conditional branches */
static void test_conditional_branches() {
	std::printf("Test 5: Conditional branches...\n");

	const char *asm_code =
		".text\n"
		"main:\n"
		"    li a0, 15\n"
		"    li a1, 10\n"
		"    blt a0, a1, less_than\n"
		"    li a0, 100     # a0 >= a1\n"
		"    j done\n"
		"less_than:\n"
		"    li a0, 200     # a0 < a1\n"
		"done:\n"
		"    li a7, 93\n"
		"    ecall\n";

	uint8_t binary[1024];
	uint32_t size;

	assert(assemble_to_memory(asm_code, binary, sizeof(binary), &size));

	auto mem = std::make_unique<Memory>(MEMORY_SIZE);
	auto cpu = std::make_unique<CPU>();

	memcpy(&mem->get_data()[0], binary, size);
	cpu->set_pc(0);

	int step_count = 0;
	while (cpu->is_running() && step_count < 1000) {
		cpu_status_t status = cpu->step(mem.get());
		step_count++;

		if (status == CPU_SYSCALL_EXIT) {
			break;
		}
		assert(status == CPU_OK);
	}

	/* Verify result: 15 >= 10, so should be 100 */
	assert(cpu->get_register(10) == 100);  /* a0 = x10 */

	std::printf("\tOK Conditional branches work (15 >= 10, result = 100)\n");
}

/* Test 6: Data section with load address */
static void test_data_section() {
	std::printf("Test 6: Data section with load address...\n");

	const char *asm_code =
		".text\n"
		"main:\n"
		"    la a0, value\n"
		"    lw a1, 0(a0)\n"
		"    lw a2, 4(a0)\n"
		"    add a0, a1, a2\n"
		"    li a7, 93\n"
		"    ecall\n"
		"\n"
		".data\n"
		"value:\n"
		"    .word 123\n"
		"    .word 456\n";

	uint8_t binary[4096];
	uint32_t size;

	assert(assemble_to_memory(asm_code, binary, sizeof(binary), &size));

	auto mem = std::make_unique<Memory>(MEMORY_SIZE);
	auto cpu = std::make_unique<CPU>();

	/* Load entire binary (text + data) into memory */
	memcpy(&mem->get_data()[0], binary, size);
	cpu->set_pc(0);

	int step_count = 0;
	while (cpu->is_running() && step_count < 10000) {
		cpu_status_t status = cpu->step(mem.get());
		step_count++;

		if (status == CPU_SYSCALL_EXIT) {
			break;
		}
		assert(status == CPU_OK);
	}

	/* Verify result: 123 + 456 = 579 */
	assert(cpu->get_register(10) == 579);  /* a0 = x10 */

	std::printf("\tOK Data section works (123 + 456 = 579)\n");
}

/* Test 7: Logical operations */
static void test_logical_operations() {
	std::printf("Test 7: Logical operations...\n");

	const char *asm_code =
		".text\n"
		"main:\n"
		"    li a0, 15\n"
		"    li a1, 10\n"
		"    and a2, a0, a1  # 15 & 10 = 10\n"
		"    or a3, a0, a1   # 15 | 10 = 15\n"
		"    xor a4, a0, a1  # 15 ^ 10 = 5\n"
		"    add a0, a2, a3  # 10 + 15 = 25\n"
		"    add a0, a0, a4  # 25 + 5 = 30\n"
		"    li a7, 93\n"
		"    ecall\n";

	uint8_t binary[1024];
	uint32_t size;

	assert(assemble_to_memory(asm_code, binary, sizeof(binary), &size));

	auto mem = std::make_unique<Memory>(MEMORY_SIZE);
	auto cpu = std::make_unique<CPU>();

	memcpy(&mem->get_data()[0], binary, size);
	cpu->set_pc(0);

	int step_count = 0;
	while (cpu->is_running() && step_count < 1000) {
		cpu_status_t status = cpu->step(mem.get());
		step_count++;

		if (status == CPU_SYSCALL_EXIT) {
			break;
		}
		assert(status == CPU_OK);
	}

	/* Verify result: 10 + 15 + 5 = 30 */
	assert(cpu->get_register(10) == 30);  /* a0 = x10 */

	std::printf("\tOK Logical operations work (result = 30)\n");
}

/* Test 8: Shift operations */
static void test_shift_operations() {
	std::printf("Test 8: Shift operations...\n");

	const char *asm_code =
		".text\n"
		"main:\n"
		"    li a0, 8\n"
		"    slli a1, a0, 2   # 8 << 2 = 32\n"
		"    srli a2, a1, 1   # 32 >> 1 = 16\n"
		"    add a0, a1, a2   # 32 + 16 = 48\n"
		"    li a7, 93\n"
		"    ecall\n";

	uint8_t binary[1024];
	uint32_t size;

	assert(assemble_to_memory(asm_code, binary, sizeof(binary), &size));

	auto mem = std::make_unique<Memory>(MEMORY_SIZE);
	auto cpu = std::make_unique<CPU>();

	memcpy(&mem->get_data()[0], binary, size);
	cpu->set_pc(0);

	int step_count = 0;
	while (cpu->is_running() && step_count < 1000) {
		cpu_status_t status = cpu->step(mem.get());
		step_count++;

		if (status == CPU_SYSCALL_EXIT) {
			break;
		}
		assert(status == CPU_OK);
	}

	/* Verify result: 32 + 16 = 48 */
	assert(cpu->get_register(10) == 48);  /* a0 = x10 */

	std::printf("\tOK Shift operations work (result = 48)\n");
}

/* Test 9: Byte and halfword operations */
static void test_byte_halfword_operations() {
	std::printf("Test 9: Byte and halfword operations...\n");

	const char *asm_code =
		".text\n"
		"main:\n"
		"    li a0, 0x2000\n"
		"    li a1, 0x42\n"
		"    sb a1, 0(a0)     # store byte\n"
		"    li a1, 234\n"
		"    sh a1, 4(a0)     # store halfword\n"
		"    lbu a2, 0(a0)    # load byte unsigned\n"
		"    lhu a3, 4(a0)    # load halfword unsigned\n"
		"    add a0, a2, a3   # 0x42 + 234 = 66 + 234 = 300\n"
		"    li a7, 93\n"
		"    ecall\n";

	uint8_t binary[1024];
	uint32_t size;

	assert(assemble_to_memory(asm_code, binary, sizeof(binary), &size));

	auto mem = std::make_unique<Memory>(MEMORY_SIZE);
	auto cpu = std::make_unique<CPU>();

	memcpy(&mem->get_data()[0], binary, size);
	cpu->set_pc(0);

	int step_count = 0;
	while (cpu->is_running() && step_count < 1000) {
		cpu_status_t status = cpu->step(mem.get());
		step_count++;

		if (status == CPU_SYSCALL_EXIT) {
			break;
		}
		assert(status == CPU_OK);
	}

	/* Verify result: 66 + 234 = 300 */
	assert(cpu->get_register(10) == 300);  /* a0 = x10 */

	std::printf("\tOK Byte/halfword operations work (result = 300)\n");
}

/* Test 10: Upper immediate instructions */
static void test_upper_immediate() {
	std::printf("Test 10: Upper immediate instructions...\n");

	const char *asm_code =
		".text\n"
		"main:\n"
		"    lui a0, 0x12345    # load upper immediate\n"
		"    addi a0, a0, 0x678 # add lower bits\n"
		"    srli a0, a0, 12    # shift right to get upper 20 bits\n"
		"    li a7, 93\n"
		"    ecall\n";

	uint8_t binary[1024];
	uint32_t size;

	assert(assemble_to_memory(asm_code, binary, sizeof(binary), &size));

	auto mem = std::make_unique<Memory>(MEMORY_SIZE);
	auto cpu = std::make_unique<CPU>();

	memcpy(&mem->get_data()[0], binary, size);
	cpu->set_pc(0);

	int step_count = 0;
	while (cpu->is_running() && step_count < 1000) {
		cpu_status_t status = cpu->step(mem.get());
		step_count++;

		if (status == CPU_SYSCALL_EXIT) {
			break;
		}
		assert(status == CPU_OK);
	}

	/* Verify result: 0x12345 */
	uint32_t result = cpu->get_register(10);
	if (result != 0x12345) {
		std::printf("\tERROR: Expected 0x12345 (%u), got 0x%x (%u)\n",
			0x12345, result, result);
	}
	assert(result == 0x12345);  /* a0 = x10 */

	std::printf("\tOK Upper immediate works (result = 0x12345)\n");
}

int main() {
	std::printf("=== RISC-V Integration Tests (Assembler + Emulator) ===\n\n");

	int test_count = 0;

	/* Run all tests */
	test_simple_arithmetic(); test_count++;
	test_loop_accumulator(); test_count++;
	test_memory_operations(); test_count++;
	test_function_call(); test_count++;
	test_conditional_branches(); test_count++;
	test_data_section(); test_count++;
	test_logical_operations(); test_count++;
	test_shift_operations(); test_count++;
	test_byte_halfword_operations(); test_count++;
	test_upper_immediate(); test_count++;

	std::printf("\n=== All %d integration tests passed! ===\n", test_count);
	return 0;
}
