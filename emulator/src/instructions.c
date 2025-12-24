// instructions.c
#include "cpu.h"
#include "instructions.h"
#include <stdint.h>

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

cpu_status_t cpu_decode(uint32_t instruction, instruction_t *decoded) {
	decoded->raw = instruction;
	decoded->imm = 0;

	/** Extract common fields */
	decoded->opcode = instruction & 0x7F;
	decoded->rd = (instruction >> 7) & 0x1F;
	decoded->funct3 = (instruction >> 12) & 0x7;
	decoded->rs1 = (instruction >> 15) & 0x1F;
	decoded->rs2 = (instruction >> 20) & 0x1F;
	decoded->funct7 = (instruction >> 25) & 0x7F;

	switch (decoded->opcode) {
		case 0x33:	/** R-type */
			decoded->format = INSTR_R_TYPE;
			decoded->imm = 0;
			break;

		case 0x03:	/** LOAD */
		case 0x13:	/** OP-IMM */
		case 0x67:	/** JALR */
		case 0x73:	/** SYSTEM */
			decoded->format = INSTR_I_TYPE;
			decoded->imm = sign_extend((instruction >> 20) & 0xFFF, 12);
			break;

		case 0x23:	/** STORE */
			decoded->format = INSTR_S_TYPE;
			decoded->imm = sign_extend(
				(((instruction >> 25) & 0x7F) << 5) |
				((instruction >> 7) & 0x1F),
				12
			);
			break;

		case 0x63:	/** BRANCH */
			decoded->format = INSTR_B_TYPE;
			decoded->imm = extract_b_imm(instruction);
			break;

		case 0x37:	/** LUI */
		case 0x17:	/** AUIPC */
			decoded->format = INSTR_U_TYPE;
			decoded->imm = (instruction & 0xFFFFF000);
			break;

		case 0x6F:	/** JAL */
			decoded->format = INSTR_J_TYPE;
			decoded->imm = extract_j_imm(instruction);
			break;

		default:
			return CPU_DECODE_ERROR;
	}

	return CPU_OK;
}
