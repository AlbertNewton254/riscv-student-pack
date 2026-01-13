/* emulator.cpp */
#include "emulator.hpp"
#include <cstdio>
#include <cstring>

Emulator::Emulator(uint32_t memory_size) {
	memory = std::make_unique<Memory>(memory_size);
	cpu = std::make_unique<CPU>();
}

CPU* Emulator::get_cpu() {
	return cpu.get();
}

const CPU* Emulator::get_cpu() const {
	return cpu.get();
}

Memory* Emulator::get_memory() {
	return memory.get();
}

const Memory* Emulator::get_memory() const {
	return memory.get();
}

cpu_status_t Emulator::step() {
	return cpu->step(memory.get());
}

int Emulator::load_program(const char *filename, uint32_t load_address) {
	FILE *file = std::fopen(filename, "rb");
	if (!file) {
		std::fprintf(stderr, "Error: Cannot open file '%s'\n", filename);
		return -1;
	}

	std::fseek(file, 0, SEEK_END);
	long file_size = std::ftell(file);
	std::fseek(file, 0, SEEK_SET);

	if (load_address + file_size > memory->get_size()) {
		std::fprintf(stderr, "Error: Program too large for memory\n");
		std::fclose(file);
		return -1;
	}

	size_t read_size = std::fread(&memory->get_data()[load_address], 1, file_size, file);
	std::fclose(file);

	if (read_size != (size_t)file_size) {
		std::fprintf(stderr, "Error: Failed to read entire file\n");
		return -1;
	}

	std::printf("Loaded %ld bytes at address 0x%08x\n", file_size, load_address);
	return 0;
}

void Emulator::set_pc(uint32_t value) {
	cpu->set_pc(value);
}

void Emulator::set_debug_mode(bool enable) {
	cpu->set_debug_mode(enable);
}

bool Emulator::is_running() const {
	return cpu->is_running();
}
