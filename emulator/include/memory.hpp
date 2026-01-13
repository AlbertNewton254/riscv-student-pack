/* memory.hpp */
#ifndef MEMORY_HPP
#define MEMORY_HPP

#include <cstdint>
#include <memory>

/*
 * Memory operation status codes
 *
 * MEM_OK: Successful operation
 * MEM_READ_ERROR: Generic read error
 * MEM_WRITE_ERROR: Generic write error
 * MEM_MISALIGNED_ERROR: Misaligned access error
 */
enum memory_status_t {
	MEM_OK,
	MEM_READ_ERROR,
	MEM_WRITE_ERROR,
	MEM_MISALIGNED_ERROR
};

/**
 * Memory class for byte-addressable memory management
 *
 * Provides aligned and unaligned read/write operations
 * for 8-bit, 16-bit, and 32-bit values
 */
class Memory {
private:
	std::unique_ptr<uint8_t[]> data;
	uint32_t size;

public:
	/**
	 * Initialize memory with given size
	 *
	 * size: Memory size in bytes
	 */
	Memory(uint32_t size);

	/**
	 * Get memory size
	 *
	 * Output: Memory size in bytes
	 */
	uint32_t get_size() const;

	/**
	 * Get raw data pointer (for syscalls)
	 *
	 * Output: Pointer to memory data
	 */
	uint8_t* get_data();

	/**
	 * Read 8-bit value from memory
	 *
	 * addr: Byte address
	 * value: Output for read value
	 *
	 * Output: Read status
	 */
	memory_status_t read8(uint32_t addr, uint8_t *value) const;

	/**
	 * Write 8-bit value to memory
	 *
	 * addr: Byte address
	 * value: Value to write
	 *
	 * Output: Write status
	 */
	memory_status_t write8(uint32_t addr, uint8_t value);

	/**
	 * Read 16-bit value from memory
	 *
	 * addr: Halfword-aligned address (addr % 2 == 0)
	 * value: Output for read value
	 *
	 * Output: Read status
	 */
	memory_status_t read16(uint32_t addr, uint16_t *value) const;

	/**
	 * Write 16-bit value to memory
	 *
	 * addr: Halfword-aligned address (addr % 2 == 0)
	 * value: Value to write
	 *
	 * Output: Write status
	 */
	memory_status_t write16(uint32_t addr, uint16_t value);

	/**
	 * Read 32-bit value from memory
	 *
	 * addr: Word-aligned address (addr % 4 == 0)
	 * value: Output for read value
	 *
	 * Output: Read status
	 */
	memory_status_t read32(uint32_t addr, uint32_t *value) const;

	/**
	 * Write 32-bit value to memory
	 *
	 * addr: Word-aligned address (addr % 4 == 0)
	 * value: Value to write
	 *
	 * Output: Write status
	 */
	memory_status_t write32(uint32_t addr, uint32_t value);
};

#endif