/* main.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cpu.h"
#include "memory.h"

static int load_program(memory_t *mem, const char *filename, uint32_t load_address) {
	FILE *file = fopen(filename, "rb");
	if (!file) {
		fprintf(stderr, "Error: Cannot open file '%s'\n", filename);
		return -1;
	}

	fseek(file, 0, SEEK_END);
	long file_size = ftell(file);
	fseek(file, 0, SEEK_SET);

	if (load_address + file_size > mem->size) {
		fprintf(stderr, "Error: Program too large for memory\n");
		fclose(file);
		return -1;
	}

	size_t read_size = fread(&mem->data[load_address], 1, file_size, file);
	fclose(file);

	if (read_size != (size_t)file_size) {
		fprintf(stderr, "Error: Failed to read entire file\n");
		return -1;
	}

	printf("Loaded %ld bytes at address 0x%08x\n", file_size, load_address);
	return 0;
}

static void dump_registers(cpu_t *cpu) {
	printf("\nRegister Dump:\n");
	printf("PC: 0x%08x\n", cpu->pc);

	printf("x0 (zero): 0x%08x\tx1 (ra): 0x%08x\tx2 (sp): 0x%08x\tx3 (gp): 0x%08x\n",
		cpu->x[0], cpu->x[1], cpu->x[2], cpu->x[3]);
	printf("x4 (tp): 0x%08x\tx5 (t0): 0x%08x\tx6 (t1): 0x%08x\tx7 (t2): 0x%08x\n",
		cpu->x[4], cpu->x[5], cpu->x[6], cpu->x[7]);
	printf("x8 (s0): 0x%08x\tx9 (s1): 0x%08x\tx10(a0): 0x%08x\tx11(a1): 0x%08x\n",
		cpu->x[8], cpu->x[9], cpu->x[10], cpu->x[11]);

	for (int i = 12; i < 32; i += 4) {
		printf("x%02d: 0x%08x\tx%02d: 0x%08x\tx%02d: 0x%08x\tx%02d: 0x%08x\n",
			i, cpu->x[i],
			i+1, cpu->x[i+1],
			i+2, cpu->x[i+2],
			i+3, cpu->x[i+3]);
	}
}

int main(int argc, char *argv[]) {
	if (argc < 2) {
		fprintf(stderr, "Usage: %s <program.bin> [load_address]\n", argv[0]);
		return 1;
	}

	uint32_t load_address = 0x00000000;
	if (argc > 2) {
		load_address = strtoul(argv[2], NULL, 0);
	}

	printf("Initializing RISC-V interpreter...\n");
	printf("Stack: 0x%08x - 0x%08x (size: %d bytes)\n",
		STACK_BASE, STACK_TOP, STACK_SIZE);

	memory_t *mem = memory_init(MEMORY_SIZE);
	if (!mem) {
		fprintf(stderr, "Error: Failed to initialize memory\n");
		return 1;
	}

	cpu_t *cpu = cpu_init();
	if (!cpu) {
		fprintf(stderr, "Error: Failed to initialize CPU\n");
		memory_destroy(mem);
		return 1;
	}

	if (load_program(mem, argv[1], load_address) != 0) {
		cpu_destroy(cpu);
		memory_destroy(mem);
		return 1;
	}

	cpu->pc = load_address;

	printf("\nStarting execution...\n");
	printf("Initial SP: 0x%08x\n", cpu->x[2]);
	printf("Initial PC: 0x%08x\n", cpu->pc);
	printf("\n");

	int step_count = 0;
	const int max_steps = 1000000;
	int exit_code = 0;

	while (cpu->running && step_count < max_steps) {
		cpu_status_t status = cpu_step(cpu, mem);
		step_count++;

		if (status == CPU_SYSCALL_EXIT) {
			exit_code = (int)cpu->x[10];
			printf("Program exited with status: %d\n", exit_code);
			break;
		}
		else if (status != CPU_OK) {
			printf("Execution stopped at step %d: Error %d\n", step_count, status);
			dump_registers(cpu);
			break;
		}

		if (step_count % 10000 == 0) {
			printf("Step %d...\n", step_count);
		}
	}

	if (step_count >= max_steps) {
		printf("Reached maximum step count (%d)\n", max_steps);
		dump_registers(cpu);
	}

	cpu_destroy(cpu);
	memory_destroy(mem);

	return exit_code;
}
