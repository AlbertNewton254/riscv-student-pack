/* emulator.hpp */
#ifndef EMULATOR_HPP
#define EMULATOR_HPP

#include <cstdint>
#include <memory>
#include "cpu.hpp"
#include "memory.hpp"

/**
 * Emulator class that owns and manages CPU and Memory
 *
 * Provides high-level interface for RISC-V emulation
 */
class Emulator {
private:
	std::unique_ptr<CPU> cpu;
	std::unique_ptr<Memory> memory;

public:
	/**
	 * Initialize emulator with given memory size
	 *
	 * memory_size: Memory size in bytes
	 */
	Emulator(uint32_t memory_size);

	/**
	 * Get CPU instance
	 *
	 * Output: Pointer to CPU
	 */
	CPU* get_cpu();

	/**
	 * Get CPU instance (const)
	 *
	 * Output: Const pointer to CPU
	 */
	const CPU* get_cpu() const;

	/**
	 * Get Memory instance
	 *
	 * Output: Pointer to Memory
	 */
	Memory* get_memory();

	/**
	 * Get Memory instance (const)
	 *
	 * Output: Const pointer to Memory
	 */
	const Memory* get_memory() const;

	/**
	 * Execute one CPU step (fetch-decode-execute)
	 *
	 * Output: Step execution status
	 */
	cpu_status_t step();

	/**
	 * Load program from file into memory
	 *
	 * filename: Path to binary file
	 * load_address: Address to load program at
	 *
	 * Output: 0 on success, -1 on error
	 */
	int load_program(const char *filename, uint32_t load_address);

	/**
	 * Set program counter
	 *
	 * value: New PC value
	 */
	void set_pc(uint32_t value);

	/**
	 * Set debug mode
	 *
	 * enable: true to enable debug output, false to disable
	 */
	void set_debug_mode(bool enable);

	/**
	 * Check if CPU is running
	 *
	 * Output: true if running, false otherwise
	 */
	bool is_running() const;
};

#endif
