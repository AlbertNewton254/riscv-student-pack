/* main.cpp */
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include "cpu.hpp"
#include "memory.hpp"

static int load_program(Memory *mem, const char *filename, uint32_t load_address) {
	FILE *file = std::fopen(filename, "rb");
	if (!file) {
		std::fprintf(stderr, "Error: Cannot open file '%s'\n", filename);
		return -1;
	}

	std::fseek(file, 0, SEEK_END);
	long file_size = std::ftell(file);
	std::fseek(file, 0, SEEK_SET);

	if (load_address + file_size > mem->get_size()) {
		std::fprintf(stderr, "Error: Program too large for memory\n");
		std::fclose(file);
		return -1;
	}

	size_t read_size = std::fread(&mem->get_data()[load_address], 1, file_size, file);
	std::fclose(file);

	if (read_size != (size_t)file_size) {
		std::fprintf(stderr, "Error: Failed to read entire file\n");
		return -1;
	}

	std::printf("Loaded %ld bytes at address 0x%08x\n", file_size, load_address);
	return 0;
}

static void dump_registers(CPU *cpu) {
	std::printf("\nRegister Dump:\n");
	std::printf("PC: 0x%08x\n", cpu->get_pc());

	std::printf("x0 (zero): 0x%08x\tx1 (ra): 0x%08x\tx2 (sp): 0x%08x\tx3 (gp): 0x%08x\n",
		cpu->get_register(0), cpu->get_register(1), cpu->get_register(2), cpu->get_register(3));
	std::printf("x4 (tp): 0x%08x\tx5 (t0): 0x%08x\tx6 (t1): 0x%08x\tx7 (t2): 0x%08x\n",
		cpu->get_register(4), cpu->get_register(5), cpu->get_register(6), cpu->get_register(7));
	std::printf("x8 (s0): 0x%08x\tx9 (s1): 0x%08x\tx10(a0): 0x%08x\tx11(a1): 0x%08x\n",
		cpu->get_register(8), cpu->get_register(9), cpu->get_register(10), cpu->get_register(11));

	for (int i = 12; i < 32; i += 4) {
		std::printf("x%02d: 0x%08x\tx%02d: 0x%08x\tx%02d: 0x%08x\tx%02d: 0x%08x\n",
			i, cpu->get_register(i),
			i+1, cpu->get_register(i+1),
			i+2, cpu->get_register(i+2),
			i+3, cpu->get_register(i+3));
	}
}

int main(int argc, char *argv[]) {
	if (argc < 2) {
		std::fprintf(stderr, "Usage: %s <program.bin> [load_address]\n", argv[0]);
		return 1;
	}

	uint32_t load_address = 0x00000000;
	if (argc > 2) {
		load_address = std::strtoul(argv[2], nullptr, 0);
	}

	std::printf("Initializing RISC-V interpreter...\n");
	std::printf("Stack: 0x%08x - 0x%08x (size: %d bytes)\n",
		STACK_BASE, STACK_TOP, STACK_SIZE);

	auto mem = std::make_unique<Memory>(MEMORY_SIZE);
	if (!mem) {
		std::fprintf(stderr, "Error: Failed to initialize memory\n");
		return 1;
	}

	auto cpu = std::make_unique<CPU>();
	if (!cpu) {
		std::fprintf(stderr, "Error: Failed to initialize CPU\n");
		return 1;
	}

	if (load_program(mem.get(), argv[1], load_address) != 0) {
		return 1;
	}

	cpu->set_pc(load_address);

	std::printf("\nStarting execution...\n");
	std::printf("Initial SP: 0x%08x\n", cpu->get_register(2));
	std::printf("Initial PC: 0x%08x\n", cpu->get_pc());
	std::printf("\n");

	int step_count = 0;
	const int max_steps = 1000000;
	int exit_code = 0;

	while (cpu->is_running() && step_count < max_steps) {
		cpu_status_t status = cpu->step(mem.get());
		step_count++;

		if (status == CPU_SYSCALL_EXIT) {
			exit_code = (int)cpu->get_register(10);
			std::printf("Program exited with status: %d\n", exit_code);
			break;
		}
		else if (status != CPU_OK) {
			std::printf("Execution stopped at step %d: Error %d\n", step_count, status);
			dump_registers(cpu.get());
			break;
		}

		if (step_count % 10000 == 0) {
			std::printf("Step %d...\n", step_count);
		}
	}

	if (step_count >= max_steps) {
		std::printf("Reached maximum step count (%d)\n", max_steps);
		dump_registers(cpu.get());
	}

	/* Smart pointers will automatically clean up memory and CPU */

	return exit_code;
}