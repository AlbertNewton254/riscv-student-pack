// cpu.h
#ifndef CPU_H
#define CPU_H

#include <stdint.h>

/**
 * CPU execution status codes
 */
typedef enum {
	CPU_OK,
	CPU_FETCH_ERROR,
	CPU_FETCH_MISALIGNED,
	CPU_FETCH_OUT_OF_BOUNDS,
	CPU_DECODE_ERROR,
	CPU_EXECUTION_ERROR,
	CPU_ILLEGAL_INSTRUCTION,
	CPU_SYSCALL_EXIT
} cpu_status_t;

/** Linux-compatible RISC-V system call numbers (RV32) */
#define SYS_exit		93
#define SYS_read		63
#define SYS_write		64
#define SYS_openat		56
#define SYS_close		57
#define SYS_brk			214
#define SYS_fstat		80
#define SYS_lseek		62

/**
 * Memory layout constants
 */
#define MEMORY_SIZE		(16 * 1024 * 1024)
#define STACK_BASE		0x80000000
#define STACK_SIZE		(1 * 1024 * 1024)
#define STACK_TOP		(STACK_BASE + STACK_SIZE)

/**
 * CPU state structure
 * x: 32 general-purpose registers (x0-x31)
 * pc: Program counter
 * running: Non-zero while CPU should continue execution
 */
typedef struct cpu_t {
	uint32_t x[32];
	uint32_t pc;
	int running;
} cpu_t;

/* Memory structure - forward declared */
typedef struct memory_t memory_t;

/* Instruction structure - defined in instructions.h */
typedef struct instruction_t instruction_t;

/**
 * Initialize CPU state
 * returns: Pointer to new CPU instance, NULL on failure
 */
cpu_t *cpu_init(void);

/**
 * Free CPU resources
 * cpu: CPU instance to destroy
 */
void cpu_destroy(cpu_t *cpu);

/**
 * Fetch next instruction from memory
 * cpu: CPU state
 * mem: Memory instance
 * instruction: Output for fetched instruction
 * returns: Fetch status
 */
cpu_status_t cpu_fetch(cpu_t *cpu, memory_t *mem, uint32_t *instruction);

/**
 * Decode raw instruction into structured format
 * instruction: Raw 32-bit instruction
 * decoded: Output for decoded instruction
 * returns: Decode status
 */
cpu_status_t cpu_decode(uint32_t instruction, instruction_t *decoded);

/**
 * Execute decoded instruction
 * cpu: CPU state
 * mem: Memory instance
 * instr: Decoded instruction to execute
 * returns: Execution status
 */
cpu_status_t cpu_execute(cpu_t *cpu, memory_t *mem, instruction_t *instr);

/**
 * Execute one CPU step (fetch-decode-execute)
 * cpu: CPU state
 * mem: Memory instance
 * returns: Step execution status
 */
cpu_status_t cpu_step(cpu_t *cpu, memory_t *mem);

#endif /* CPU_H */
