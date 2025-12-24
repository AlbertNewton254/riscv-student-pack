#include "assembler.h"

uint32_t encode_r(uint32_t funct7, uint32_t rs2, uint32_t rs1,
				  uint32_t funct3, uint32_t rd, uint32_t opcode) {
	return (funct7 << 25) | (rs2 << 20) | (rs1 << 15) |
		   (funct3 << 12) | (rd << 7) | opcode;
}

uint32_t encode_i(int32_t imm, uint32_t rs1,
				  uint32_t funct3, uint32_t rd, uint32_t opcode) {
	return ((imm & 0xFFF) << 20) | (rs1 << 15) |
		   (funct3 << 12) | (rd << 7) | opcode;
}

uint32_t encode_s(int32_t imm, uint32_t rs2, uint32_t rs1,
				  uint32_t funct3, uint32_t opcode) {
	return ((imm & 0xFE0) << 20) |
		   (rs2 << 20) | (rs1 << 15) |
		   (funct3 << 12) |
		   ((imm & 0x1F) << 7) |
		   opcode;
}

uint32_t encode_b(int32_t imm, uint32_t rs2, uint32_t rs1,
				  uint32_t funct3, uint32_t opcode) {
	return ((imm & 0x1000) << 19) |
		   ((imm & 0x7E0) << 20) |
		   (rs2 << 20) | (rs1 << 15) |
		   (funct3 << 12) |
		   ((imm & 0x1E) << 7) |
		   ((imm & 0x800) >> 4) |
		   opcode;
}

uint32_t encode_u(int32_t imm, uint32_t rd, uint32_t opcode) {
	return (imm & 0xFFFFF000) | (rd << 7) | opcode;
}

uint32_t encode_j(int32_t imm, uint32_t rd, uint32_t opcode) {
	return ((imm & 0x100000) << 11) |
		   ((imm & 0x7FE) << 20) |
		   ((imm & 0x800) << 9) |
		   (imm & 0xFF000) |
		   (rd << 7) |
		   opcode;
}
