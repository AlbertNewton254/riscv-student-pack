// memory.h
#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>

/**
 * Memory instance structure
 * data: Byte-addressable memory array
 * size: Total memory size in bytes
 */
typedef struct memory_t {
	uint8_t *data;
	uint32_t size;
} memory_t;

/** Memory operation status codes */
typedef enum {
	MEM_OK,
	MEM_READ_ERROR,
	MEM_WRITE_ERROR,
	MEM_MISALIGNED_ERROR
} memory_status_t;

/**
 * Initialize memory with given size
 * size: Memory size in bytes
 * returns: Pointer to new memory instance, NULL on failure
 */
memory_t *memory_init(uint32_t size);

/**
 * Free memory resources
 * mem: Memory instance to destroy
 */
void memory_destroy(memory_t *mem);

/**
 * Read 8-bit value from memory
 * mem: Memory instance
 * addr: Byte address
 * value: Output for read value
 * returns: Read status
 */
memory_status_t memory_read8(memory_t *mem, uint32_t addr, uint8_t *value);

/**
 * Write 8-bit value to memory
 * mem: Memory instance
 * addr: Byte address
 * value: Value to write
 * returns: Write status
 */
memory_status_t memory_write8(memory_t *mem, uint32_t addr, uint8_t value);

/**
 * Read 16-bit value from memory
 * mem: Memory instance
 * addr: Halfword-aligned address (addr % 2 == 0)
 * value: Output for read value
 * returns: Read status
 */
memory_status_t memory_read16(memory_t *mem, uint32_t addr, uint16_t *value);

/**
 * Write 16-bit value to memory
 * mem: Memory instance
 * addr: Halfword-aligned address (addr % 2 == 0)
 * value: Value to write
 * returns: Write status
 */
memory_status_t memory_write16(memory_t *mem, uint32_t addr, uint16_t value);

/**
 * Read 32-bit value from memory
 * mem: Memory instance
 * addr: Word-aligned address (addr % 4 == 0)
 * value: Output for read value
 * returns: Read status
 */
memory_status_t memory_read32(memory_t *mem, uint32_t addr, uint32_t *value);

/**
 * Write 32-bit value to memory
 * mem: Memory instance
 * addr: Word-aligned address (addr % 4 == 0)
 * value: Value to write
 * returns: Write status
 */
memory_status_t memory_write32(memory_t *mem, uint32_t addr, uint32_t value);

#endif /* MEMORY_H */
