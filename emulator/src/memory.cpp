/* memory.cpp */
#include "memory.hpp"
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <memory>

Memory::Memory(uint32_t size) : size(size) {
	data = std::make_unique<uint8_t[]>(size);

	/* Zero-initialize memory */
	std::memset(data.get(), 0, size);
}

uint32_t Memory::get_size() const {
	return size;
}

uint8_t* Memory::get_data() {
	return data.get();
}

memory_status_t Memory::read8(uint32_t addr, uint8_t *value) const {
	if (addr >= size) {
		return MEM_READ_ERROR;
	}

	*value = data[addr];

	return MEM_OK;
}

memory_status_t Memory::write8(uint32_t addr, uint8_t value) {
	if (addr >= size) {
		return MEM_WRITE_ERROR;
	}

	data[addr] = value;

	return MEM_OK;
}

memory_status_t Memory::read16(uint32_t addr, uint16_t *value) const {
	if (addr % 2 != 0) {
		return MEM_MISALIGNED_ERROR;
	}

	/* Check for overflow and bounds */
	if (addr > size - 2) {
		return MEM_READ_ERROR;
	}

	*value = (uint16_t)((data[addr]) | (data[addr + 1] << 8));

	return MEM_OK;
}

memory_status_t Memory::write16(uint32_t addr, uint16_t value) {
	if (addr % 2 != 0) {
		return MEM_MISALIGNED_ERROR;
	}

	/* Check for overflow and bounds */
	if (addr > size - 2) {
		return MEM_WRITE_ERROR;
	}

	data[addr] = (uint8_t)(value & 0xFF);
	data[addr + 1] = (uint8_t)((value >> 8) & 0xFF);

	return MEM_OK;
}

memory_status_t Memory::read32(uint32_t addr, uint32_t *value) const {
	if (addr % 4 != 0) {
		return MEM_MISALIGNED_ERROR;
	}

	/* Check for overflow and bounds */
	if (addr > size - 4) {
		return MEM_READ_ERROR;
	}

	*value = (uint32_t)((data[addr]) |
		(data[addr + 1] << 8) |
		(data[addr + 2] << 16) |
		(data[addr + 3] << 24));

	return MEM_OK;
}

memory_status_t Memory::write32(uint32_t addr, uint32_t value) {
	if (addr % 4 != 0) {
		return MEM_MISALIGNED_ERROR;
	}

	/* Check for overflow and bounds */
	if (addr > size - 4) {
		return MEM_WRITE_ERROR;
	}

	data[addr] = (uint8_t)(value & 0xFF);
	data[addr + 1] = (uint8_t)((value >> 8) & 0xFF);
	data[addr + 2] = (uint8_t)((value >> 16) & 0xFF);
	data[addr + 3] = (uint8_t)((value >> 24) & 0xFF);

	return MEM_OK;
}