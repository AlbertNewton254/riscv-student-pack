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

	if (read_size != (size_t)file_size) {  /* Fixed signedness warning */
		fprintf(stderr, "Error: Failed to read entire file\n");
		return -1;
	}

	printf("Loaded %ld bytes at address 0x%08x\n", file_size, load_address);
	return 0;
}

static void dump_registers(cpu_t *cpu) {
	printf("\nRegister Dump:\n");
	printf("PC: 0x%08x\n", cpu->pc);

	for (int i = 0; i < 32; i += 4) {
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

	memory_t *mem = memory_init(16 * 1024 * 1024);
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

	printf("Starting execution...\n");
	int step_count = 0;
	const int max_steps = 1000000;

	while (cpu->running && step_count < max_steps) {
		cpu_status_t status = cpu_step(cpu, mem);
		step_count++;

		if (status == CPU_SYSCALL_EXIT) {
			printf("Program exited with status: %d\n", cpu->x[10]);
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

	return 0;
}