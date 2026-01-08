/* encode.cpp */
#include "assembler.hpp"

uint32_t Encoder::encode_r(uint32_t funct7, uint32_t rs2, uint32_t rs1, uint32_t funct3, uint32_t rd, uint32_t opcode) {
	return (funct7 << 25) | (rs2 << 20) | (rs1 << 15) | (funct3 << 12) | (rd << 7) | opcode;
}

uint32_t Encoder::encode_i(int32_t imm, uint32_t rs1, uint32_t funct3, uint32_t rd, uint32_t opcode) {
	return ((imm & 0xFFF) << 20) | (rs1 << 15) | (funct3 << 12) | (rd << 7) | opcode;
}

uint32_t Encoder::encode_s(int32_t imm, uint32_t rs2, uint32_t rs1, uint32_t funct3, uint32_t opcode) {
	return ((imm & 0xFE0) << 20) | (rs2 << 20) | (rs1 << 15) | (funct3 << 12) | ((imm & 0x1F) << 7) | opcode;
}

uint32_t Encoder::encode_b(int32_t imm, uint32_t rs2, uint32_t rs1, uint32_t funct3, uint32_t opcode) {
	uint32_t u = (uint32_t)(imm >> 1);
	return (((u >> 11) & 0x1) << 31) |
		(((u >> 4)  & 0x3F) << 25) |
		(rs2 << 20) | (rs1 << 15) |
		(funct3 << 12) |
		((u & 0xF) << 8) |
		(((u >> 10) & 0x1) << 7) |
		opcode;
}

uint32_t Encoder::encode_u(int32_t imm, uint32_t rd, uint32_t opcode) {
	return (imm & 0xFFFFF000) | (rd << 7) | opcode;
}

uint32_t Encoder::encode_j(int32_t imm, uint32_t rd, uint32_t opcode) {
	uint32_t u = (uint32_t)(imm >> 1);

	return (((u >> 19) & 0x1) << 31) |
		((u  & 0x3FF) << 21) |
		(((u >> 10) & 0x1) << 20) |
		(((u >> 11) & 0xFF) << 12) |
		(rd << 7) |
		opcode;
}