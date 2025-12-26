#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <stdint.h>
#include <stdio.h>

/* Maximum label table size */
#define MAX_LABELS 256

/* Maximum line length for assembly source */
#define MAX_LINE	512

/**
 * Assembly section types
 * SEC_TEXT: Executable code section
 * SEC_DATA: Data section (constants, strings, etc.)
 */
typedef enum {
	SEC_TEXT,
	SEC_DATA
} section_t;

/**
 * Label definition structure
 * name: Label identifier (max 63 chars + null terminator)
 * addr: Absolute address assigned to label
 */
typedef struct {
	char name[64];
	uint32_t addr;
} label_t;

/**
 * Assembler global state structure
 * labels: Table of label definitions
 * label_count: Number of labels currently defined
 * pc_text: Program counter for text section
 * pc_data: Program counter for data section
 * current_section: Active section during parsing
 */
typedef struct {
	label_t labels[MAX_LABELS];
	int label_count;
	uint32_t pc_text;
	uint32_t pc_data;
	section_t current_section;
} assembler_state_t;

/**
 * Remove leading and trailing whitespace from string
 * s: String to trim (modified in-place)
 * returns: Pointer to trimmed string
 */
char *trim(char *s);

/**
 * Convert register name string to register number
 * r: Register name (e.g., "x1", "x31")
 * returns: Register number (0-31) or -1 if invalid
 */
int reg_num(const char *r);

/**
 * Look up label address by name
 * state: Assembler state containing label table
 * name: Label name to search for
 * returns: Address associated with label
 * exits: If label not found
 */
uint32_t find_label(const assembler_state_t *state, const char *name);

/**
 * Parse immediate value or label
 * state: Assembler state (for label lookup)
 * s: String containing immediate value or label name
 * returns: Parsed 32-bit immediate value
 */
int32_t parse_imm(const assembler_state_t *state, const char *s);

/**
 * Encode R-type (register) instruction
 * funct7: 7-bit function code (upper bits)
 * rs2: Source register 2 (5 bits)
 * rs1: Source register 1 (5 bits)
 * funct3: 3-bit function code
 * rd: Destination register (5 bits)
 * opcode: 7-bit opcode
 * returns: Encoded 32-bit instruction
 */
uint32_t encode_r(uint32_t funct7, uint32_t rs2, uint32_t rs1,
				  uint32_t funct3, uint32_t rd, uint32_t opcode);

/**
 * Encode I-type (immediate) instruction
 * imm: 12-bit immediate value (sign-extended)
 * rs1: Source register 1 (5 bits)
 * funct3: 3-bit function code
 * rd: Destination register (5 bits)
 * opcode: 7-bit opcode
 * returns: Encoded 32-bit instruction
 */
uint32_t encode_i(int32_t imm, uint32_t rs1,
				  uint32_t funct3, uint32_t rd, uint32_t opcode);

/**
 * Encode S-type (store) instruction
 * imm: 12-bit immediate value (split into two fields)
 * rs2: Source register 2 (5 bits)
 * rs1: Source register 1 (5 bits)
 * funct3: 3-bit function code
 * opcode: 7-bit opcode
 * returns: Encoded 32-bit instruction
 */
uint32_t encode_s(int32_t imm, uint32_t rs2, uint32_t rs1,
				  uint32_t funct3, uint32_t opcode);

/**
 * Encode B-type (branch) instruction
 * imm: 13-bit immediate value (word-aligned, split into multiple fields)
 * rs2: Source register 2 (5 bits)
 * rs1: Source register 1 (5 bits)
 * funct3: 3-bit function code
 * opcode: 7-bit opcode
 * returns: Encoded 32-bit instruction
 */
uint32_t encode_b(int32_t imm, uint32_t rs2, uint32_t rs1,
				  uint32_t funct3, uint32_t opcode);

/**
 * Encode U-type (upper immediate) instruction
 * imm: 20-bit immediate value (aligned to 12-bit boundary)
 * rd: Destination register (5 bits)
 * opcode: 7-bit opcode
 * returns: Encoded 32-bit instruction
 */
uint32_t encode_u(int32_t imm, uint32_t rd, uint32_t opcode);

/**
 * Encode J-type (jump) instruction
 * imm: 21-bit immediate value (word-aligned, split into multiple fields)
 * rd: Destination register (5 bits)
 * opcode: 7-bit opcode
 * returns: Encoded 32-bit instruction
 */
uint32_t encode_j(int32_t imm, uint32_t rd, uint32_t opcode);


/**
 * Parse C-style escaped string literal.
 *
 * Supports:
 *   \n  newline
 *   \t  tab
 *   \r  carriage return
 *   \\  backslash
 *   \"  double quote
 *
 * src: Pointer to first char AFTER opening quote
 * out: Output buffer (NULL = count only)
 * returns: Number of bytes produced
 */
size_t parse_escaped_string(const char *src, uint8_t *out);

/**
 * First assembly pass: label resolution and address calculation
 * f: Input assembly file stream
 * state: Assembler state to initialize and populate
 * 
 * Performs:
 * 1. Section tracking (.text, .data)
 * 2. Label address assignment
 * 3. Address calculation for text and data sections
 * 4. Data label relocation (adjusting for text section size)
 */
void first_pass(FILE *f, assembler_state_t *state);

/**
 * Second assembly pass: instruction encoding and output generation
 * in: Input assembly file stream (rewound after first pass)
 * out: Output binary file stream
 * state: Assembler state from first pass
 * 
 * Performs:
 * 1. Instruction parsing and encoding
 * 2. Data directive processing (.ascii, .byte)
 * 3. Binary output to appropriate file positions
 * 4. Label reference resolution
 */
void second_pass(FILE *in, FILE *out, const assembler_state_t *state);

#endif /* ASSEMBLER_H */
