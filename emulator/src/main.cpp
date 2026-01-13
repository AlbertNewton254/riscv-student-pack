/* main.cpp */
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include "emulator.hpp"
#include "cpu.hpp"

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
	bool debug_mode = false;
	const char *program_file = nullptr;
	uint32_t load_address = 0x00000000;

	/* Parse command line arguments */
	for (int i = 1; i < argc; i++) {
		if (std::strcmp(argv[i], "--debug") == 0) {
			debug_mode = true;
		} else if (!program_file) {
			program_file = argv[i];
		} else {
			load_address = std::strtoul(argv[i], nullptr, 0);
		}
	}

	if (!program_file) {
		std::fprintf(stderr, "Usage: %s [--debug] <program.bin> [load_address]\n", argv[0]);
		return 1;
	}

	std::printf("Initializing RISC-V interpreter...\n");
	std::printf("Stack: 0x%08x - 0x%08x (size: %d bytes)\n",
		STACK_BASE, STACK_TOP, STACK_SIZE);

	auto emulator = std::make_unique<Emulator>(MEMORY_SIZE);
	if (!emulator) {
		std::fprintf(stderr, "Error: Failed to initialize emulator\n");
		return 1;
	}

	if (emulator->load_program(program_file, load_address) != 0) {
		return 1;
	}

	emulator->set_pc(load_address);
	emulator->set_debug_mode(debug_mode);

	std::printf("\nStarting execution...\n");
	std::printf("Initial SP: 0x%08x\n", emulator->get_cpu()->get_register(2));
	std::printf("Initial PC: 0x%08x\n", emulator->get_cpu()->get_pc());
	std::printf("\n");

	int step_count = 0;
	const int max_steps = 1000000;
	int exit_code = 0;

	while (emulator->is_running() && step_count < max_steps) {
		cpu_status_t status = emulator->step();
		step_count++;

		if (status == CPU_SYSCALL_EXIT) {
			exit_code = (int)emulator->get_cpu()->get_register(10);
			std::printf("Program exited with status: %d\n", exit_code);
			break;
		}
		else if (status != CPU_OK) {
			std::printf("Execution stopped at step %d: Error %d\n", step_count, status);
			dump_registers(emulator->get_cpu());
			break;
		}

		if (step_count % 10000 == 0) {
			std::printf("Step %d...\n", step_count);
		}
	}

	if (step_count >= max_steps) {
		std::printf("Reached maximum step count (%d)\n", max_steps);
		dump_registers(emulator->get_cpu());
	}

	/* Smart pointers will automatically clean up emulator */

	return exit_code;
}