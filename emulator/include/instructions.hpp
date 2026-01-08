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

/**
 * Decoded instruction class
 *
 * Encapsulates instruction decoding and field access
 */
class Instruction {
private:
	instr_format_t format;
	uint32_t raw;
	int32_t imm;
	uint8_t opcode;
	uint8_t rd;
	uint8_t rs1;
	uint8_t rs2;
	uint8_t funct3;
	uint8_t funct7;

public:
	/**
	 * Decode raw 32-bit instruction
	 *
	 * instruction: Raw 32-bit instruction word
	 *
	 * Output: true if successfully decoded, false otherwise
	 */
	bool decode(uint32_t instruction);

	/* Getters */
	instr_format_t get_format() const;
	uint32_t get_raw() const;
	int32_t get_imm() const;
	uint8_t get_opcode() const;
	uint8_t get_rd() const;
	uint8_t get_rs1() const;
	uint8_t get_rs2() const;
	uint8_t get_funct3() const;
	uint8_t get_funct7() const;
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