// instructions.h
#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

#include <stdint.h>

/** Instruction format types */
typedef enum {
	INSTR_R_TYPE,
	INSTR_I_TYPE,
	INSTR_S_TYPE,
	INSTR_B_TYPE,
	INSTR_U_TYPE,
	INSTR_J_TYPE
} instr_format_t;

/**
 * Decoded instruction structure
 */
typedef struct instruction_t {
	instr_format_t format;
	uint32_t raw;      /** Raw instruction */
	int32_t imm;       /** Sign-extended immediate */

	/** Extracted fields */
	uint8_t opcode;
	uint8_t rd;
	uint8_t rs1;
	uint8_t rs2;
	uint8_t funct3;
	uint8_t funct7;
} instruction_t;

/**
 * Sign extend a value to 32 bits
 * value: Value to sign extend
 * bits: Original bit width
 * returns: 32-bit sign-extended value
 */
int32_t sign_extend(uint32_t value, int bits);

/* cpu_decode is declared in cpu.h but defined in instructions.c.
   The prototype in cpu.h is sufficient for external users. */

#endif /* INSTRUCTIONS_H */
