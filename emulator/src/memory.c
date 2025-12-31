/* memory.c */
#include "memory.h"
#include <stdlib.h>
#include <stdint.h>

memory_t *memory_init(uint32_t size) {
	memory_t *mem = (memory_t *)malloc(sizeof(memory_t));
	if (!mem) {
		return NULL;
	}

	mem->data = (uint8_t *)calloc(size, sizeof(uint8_t));
	if (!mem->data) {
		free(mem);
		return NULL;
	}

	mem->size = size;
	return mem;
}

void memory_destroy(memory_t *mem) {
	if (mem) {
		free(mem->data);
		free(mem);
	}
}

memory_status_t memory_read8(memory_t *mem, uint32_t addr, uint8_t *value) {
	if (addr >= mem->size) {
		return MEM_READ_ERROR;
	}

	*value = mem->data[addr];

	return MEM_OK;
}

memory_status_t memory_write8(memory_t *mem, uint32_t addr, uint8_t value) {
	if (addr >= mem->size) {
		return MEM_WRITE_ERROR;
	}

	mem->data[addr] = value;

	return MEM_OK;
}

memory_status_t memory_read16(memory_t *mem, uint32_t addr, uint16_t *value) {
	if (addr % 2 != 0) {
		return MEM_MISALIGNED_ERROR;
	}

	if (addr + 1 >= mem->size) {
		return MEM_READ_ERROR;
	}

	*value = (uint16_t)((mem->data[addr]) | (mem->data[addr + 1] << 8));

	return MEM_OK;
}

memory_status_t memory_write16(memory_t *mem, uint32_t addr, uint16_t value) {
	if (addr % 2 != 0) {
		return MEM_MISALIGNED_ERROR;
	}

	if (addr + 1 >= mem->size) {
		return MEM_WRITE_ERROR;
	}

	mem->data[addr] = (uint8_t)(value & 0xFF);
	mem->data[addr + 1] = (uint8_t)((value >> 8) & 0xFF);

	return MEM_OK;
}

memory_status_t memory_read32(memory_t *mem, uint32_t addr, uint32_t *value) {
	if (addr % 4 != 0) {
		return MEM_MISALIGNED_ERROR;
	}

	if (addr + 3 >= mem->size) {
		return MEM_READ_ERROR;
	}

	*value = (uint32_t)((mem->data[addr]) |
		(mem->data[addr + 1] << 8) |
		(mem->data[addr + 2] << 16) |
		(mem->data[addr + 3] << 24));

	return MEM_OK;
}

memory_status_t memory_write32(memory_t *mem, uint32_t addr, uint32_t value) {
	if (addr % 4 != 0) {
		return MEM_MISALIGNED_ERROR;
	}

	if (addr + 3 >= mem->size) {
		return MEM_WRITE_ERROR;
	}

	mem->data[addr] = (uint8_t)(value & 0xFF);
	mem->data[addr + 1] = (uint8_t)((value >> 8) & 0xFF);
	mem->data[addr + 2] = (uint8_t)((value >> 16) & 0xFF);
	mem->data[addr + 3] = (uint8_t)((value >> 24) & 0xFF);

	return MEM_OK;
}
