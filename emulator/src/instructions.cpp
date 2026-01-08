/* instructions.cpp */
#include "cpu.hpp"
#include "instructions.hpp"
#include <cstdint>

int32_t sign_extend(uint32_t value, int bits) {
	/* sign-extend 'value' that has 'bits' significant bits */
	if (bits <= 0 || bits >= 32) {
		return (int32_t)value;
	}
	uint32_t mask = 1u << (bits - 1);
	int32_t x = (int32_t)value;
	return (int32_t)((x ^ (int32_t)mask) - (int32_t)mask);
}

static int32_t extract_b_imm(uint32_t instruction) {
	int32_t imm = (((instruction >> 31) & 0x1) << 12) |
				(((instruction >> 7) & 0x1) << 11) |
				(((instruction >> 25) & 0x3F) << 5) |
				(((instruction >> 8) & 0xF) << 1);
	return sign_extend((uint32_t)imm, 13);
}

static int32_t extract_j_imm(uint32_t instruction) {
	int32_t imm = (((instruction >> 31) & 0x1) << 20) |
				(((instruction >> 12) & 0xFF) << 12) |
				(((instruction >> 20) & 0x1) << 11) |
				(((instruction >> 21) & 0x3FF) << 1);
	return sign_extend((uint32_t)imm, 21);
}

bool Instruction::decode(uint32_t instruction) {
	raw = instruction;
	imm = 0;

	/* Extract common fields */
	opcode = instruction & 0x7F;
	rd = (instruction >> 7) & 0x1F;
	funct3 = (instruction >> 12) & 0x7;
	rs1 = (instruction >> 15) & 0x1F;
	rs2 = (instruction >> 20) & 0x1F;
	funct7 = (instruction >> 25) & 0x7F;

	switch (opcode) {
		case 0x33:	/* R-type */
			format = INSTR_R_TYPE;
			imm = 0;
			break;

		case 0x03:	/* LOAD */
		case 0x13:	/* OP-IMM */
		case 0x67:	/* JALR */
		case 0x73:	/* SYSTEM */
			format = INSTR_I_TYPE;
			imm = sign_extend((instruction >> 20) & 0xFFF, 12);
			break;

		case 0x23:	/* STORE */
			format = INSTR_S_TYPE;
			imm = sign_extend(
				(((instruction >> 25) & 0x7F) << 5) |
				((instruction >> 7) & 0x1F),
				12
			);
			break;

		case 0x63:	/* BRANCH */
			format = INSTR_B_TYPE;
			imm = extract_b_imm(instruction);
			break;

		case 0x37:	/* LUI */
		case 0x17:	/* AUIPC */
			format = INSTR_U_TYPE;
			imm = (instruction & 0xFFFFF000);
			break;

		case 0x6F:	/* JAL */
			format = INSTR_J_TYPE;
			imm = extract_j_imm(instruction);
			break;

		default:
			return false;
	}

	return true;
}

instr_format_t Instruction::get_format() const {
	return format;
}

uint32_t Instruction::get_raw() const {
	return raw;
}

int32_t Instruction::get_imm() const {
	return imm;
}

uint8_t Instruction::get_opcode() const {
	return opcode;
}

uint8_t Instruction::get_rd() const {
	return rd;
}

uint8_t Instruction::get_rs1() const {
	return rs1;
}

uint8_t Instruction::get_rs2() const {
	return rs2;
}

uint8_t Instruction::get_funct3() const {
	return funct3;
}

uint8_t Instruction::get_funct7() const {
	return funct7;
}