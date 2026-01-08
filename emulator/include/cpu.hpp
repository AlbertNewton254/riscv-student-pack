/* cpu.hpp */
#ifndef CPU_HPP
#define CPU_HPP

#include <cstdint>
#include <memory>

/* Forward declarations */
class Memory;
class Instruction;

/*
 * CPU execution status codes
 *
 * CPU_OK: Successful execution
 * CPU_FETCH_ERROR: Generic fetch error
 * CPU_FETCH_MISALIGNED: Misaligned instruction fetch
 * CPU_FETCH_OUT_OF_BOUNDS: Instruction fetch out of bounds
 * CPU_DECODE_ERROR: Generic decode error
 * CPU_EXECUTION_ERROR: Generic execution error
 * CPU_ILLEGAL_INSTRUCTION: Illegal instruction encountered
 * CPU_SYSCALL_EXIT: System call exit requested
 */
enum cpu_status_t {
	CPU_OK,
	CPU_FETCH_ERROR,
	CPU_FETCH_MISALIGNED,
	CPU_FETCH_OUT_OF_BOUNDS,
	CPU_DECODE_ERROR,
	CPU_EXECUTION_ERROR,
	CPU_ILLEGAL_INSTRUCTION,
	CPU_SYSCALL_EXIT
};

/* Linux-compatible RISC-V system call numbers (RV32) */
#define SYS_exit 93
#define SYS_read 63
#define SYS_write 64
#define SYS_openat 56
#define SYS_close 57
#define SYS_brk 214
#define SYS_fstat 80
#define SYS_lseek 62

/*
 * Memory layout constants
 */
#define MEMORY_SIZE (16 * 1024 * 1024)
#define STACK_BASE 0x80000000
#define STACK_SIZE (1 * 1024 * 1024)
#define STACK_TOP (STACK_BASE + STACK_SIZE)

/**
 * CPU class for RISC-V processor emulation
 *
 * Implements fetch-decode-execute cycle and register file management
 */
class CPU {
private:
	uint32_t x[32];
	uint32_t pc;
	bool running;

	/**
	 * Read register value (x0 always returns 0)
	 *
	 * reg: Register number (0-31)
	 *
	 * Output: Register value
	 */
	uint32_t reg_read(uint8_t reg);

	/**
	 * Write register value (x0 writes are ignored)
	 *
	 * reg: Register number (0-31)
	 * value: Value to write
	 */
	void reg_write(uint8_t reg, uint32_t value);

	/**
	 * Handle system call instruction
	 *
	 * mem: Memory instance
	 *
	 * Output: Syscall status
	 */
	cpu_status_t handle_syscall(Memory *mem);

	/**
	 * Execute load instruction
	 *
	 * mem: Memory instance
	 * instr: Decoded instruction
	 * result: Output for loaded value
	 *
	 * Output: Load status
	 */
	cpu_status_t execute_load(Memory *mem, Instruction *instr, uint32_t *result);

	/**
	 * Execute store instruction
	 *
	 * mem: Memory instance
	 * instr: Decoded instruction
	 *
	 * Output: Store status
	 */
	cpu_status_t execute_store(Memory *mem, Instruction *instr);

	/**
	 * Execute ALU operation
	 *
	 * rs1_val: First operand value
	 * rs2_val_or_imm: Second operand value or immediate
	 * funct3: 3-bit function code
	 * funct7: 7-bit function code
	 * is_imm: True if second operand is immediate
	 *
	 * Output: ALU result
	 */
	uint32_t execute_alu(uint32_t rs1_val, uint32_t rs2_val_or_imm, uint8_t funct3, uint8_t funct7, bool is_imm);

	/**
	 * Execute branch instruction
	 *
	 * instr: Decoded instruction
	 *
	 * Output: Branch status
	 */
	cpu_status_t execute_branch(Instruction *instr);

	/**
	 * Execute system instruction
	 *
	 * mem: Memory instance
	 * instr: Decoded instruction
	 *
	 * Output: System instruction status
	 */
	cpu_status_t execute_system(Memory *mem, Instruction *instr);

public:
	/**
	 * Initialize CPU state
	 */
	CPU();

	/**
	 * Check if CPU is running
	 *
	 * Output: true if running, false otherwise
	 */
	bool is_running() const;

	/**
	 * Get program counter
	 *
	 * Output: Current PC value
	 */
	uint32_t get_pc() const;

	/**
	 * Set program counter
	 *
	 * value: New PC value
	 */
	void set_pc(uint32_t value);

	/**
	 * Get register value (for testing/debugging)
	 *
	 * reg: Register number (0-31)
	 *
	 * Output: Register value
	 */
	uint32_t get_register(uint8_t reg) const;

	/**
	 * Set register value (for testing/setup)
	 *
	 * reg: Register number (0-31)
	 * value: Value to write
	 */
	void set_register(uint8_t reg, uint32_t value);

	/**
	 * Fetch next instruction from memory
	 *
	 * mem: Memory instance
	 * instruction: Output for fetched instruction
	 *
	 * Output: Fetch status
	 */
	cpu_status_t fetch(Memory *mem, uint32_t *instruction);

	/**
	 * Execute decoded instruction
	 *
	 * mem: Memory instance
	 * instr: Decoded instruction to execute
	 *
	 * Output: Execution status
	 */
	cpu_status_t execute(Memory *mem, Instruction *instr);

	/**
	 * Execute one CPU step (fetch-decode-execute)
	 *
	 * mem: Memory instance
	 *
	 * Output: Step execution status
	 */
	cpu_status_t step(Memory *mem);
};

#endif