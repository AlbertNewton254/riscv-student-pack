/* memory.hpp */
#ifndef MEMORY_HPP
#define MEMORY_HPP

#include <cstdint>
#include <memory>

/*
 * Memory instance structure
 *
 * data: Byte-addressable memory array (managed by unique_ptr)
 * size: Total memory size in bytes
 */
struct memory_t {
	std::unique_ptr<uint8_t[]> data;
	uint32_t size;
};

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
 * Initialize memory with given size
 *
 * size: Memory size in bytes
 *
 * Output: unique_ptr to new memory instance, nullptr on failure
 */
std::unique_ptr<memory_t> memory_init(uint32_t size);

/**
 * Read 8-bit value from memory
 *
 * mem: Memory instance
 * addr: Byte address
 * value: Output for read value
 *
 * Output: Read status
 */
memory_status_t memory_read8(memory_t *mem, uint32_t addr, uint8_t *value);

/**
 * Write 8-bit value to memory
 *
 * mem: Memory instance
 * addr: Byte address
 * value: Value to write
 *
 * Output: Write status
 */
memory_status_t memory_write8(memory_t *mem, uint32_t addr, uint8_t value);

/**
 * Read 16-bit value from memory
 *
 * mem: Memory instance
 * addr: Halfword-aligned address (addr % 2 == 0)
 * value: Output for read value
 *
 * Output: Read status
 */
memory_status_t memory_read16(memory_t *mem, uint32_t addr, uint16_t *value);

/**
 * Write 16-bit value to memory
 *
 * mem: Memory instance
 * addr: Halfword-aligned address (addr % 2 == 0)
 * value: Value to write
 *
 * Output: Write status
 */
memory_status_t memory_write16(memory_t *mem, uint32_t addr, uint16_t value);

/**
 * Read 32-bit value from memory
 *
 * mem: Memory instance
 * addr: Word-aligned address (addr % 4 == 0)
 * value: Output for read value
 *
 * Output: Read status
 */
memory_status_t memory_read32(memory_t *mem, uint32_t addr, uint32_t *value);

/**
 * Write 32-bit value to memory
 *
 * mem: Memory instance
 * addr: Word-aligned address (addr % 4 == 0)
 * value: Value to write
 *
 * Output: Write status
 */
memory_status_t memory_write32(memory_t *mem, uint32_t addr, uint32_t value);

#endif