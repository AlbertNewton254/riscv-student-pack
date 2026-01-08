/* instructions.hpp */
#ifndef INSTRUCTIONS_HPP
#define INSTRUCTIONS_HPP

#include <cstdint>

/*
 * Instruction format types
 *
 * INSTR_R_TYPE: Register-type instructions
 * INSTR_I_TYPE: Immediate-type instructions
 * INSTR_S_TYPE: Store-type instructions
 * INSTR_B_TYPE: Branch-type instructions
 * INSTR_U_TYPE: Upper-immediate-type instructions
 * INSTR_J_TYPE: Jump-type instructions
 */
enum instr_format_t {
	INSTR_R_TYPE,
	INSTR_I_TYPE,
	INSTR_S_TYPE,
	INSTR_B_TYPE,
	INSTR_U_TYPE,
	INSTR_J_TYPE
};

/*
 * Decoded instruction structure
 *
 * format: Instruction format type
 * raw: Raw 32-bit instruction word
 * imm: Sign-extended immediate value
 * opcode: 7-bit opcode field
 * rd: Destination register field
 * rs1: Source register 1 field
 * rs2: Source register 2 field
 * funct3: 3-bit function field
 * funct7: 7-bit function field
 */
struct instruction_t {
	instr_format_t format;
	uint32_t raw;
	int32_t imm;

	uint8_t opcode;
	uint8_t rd;
	uint8_t rs1;
	uint8_t rs2;
	uint8_t funct3;
	uint8_t funct7;
};

/**
 * Sign extend a value to 32 bits
 *
 * value: Value to sign extend
 * bits: Original bit width
 *
 * Output: 32-bit sign-extended value
 */
int32_t sign_extend(uint32_t value, int bits);

#endif