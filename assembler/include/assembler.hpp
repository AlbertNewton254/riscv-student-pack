/* assembler.hpp */
#ifndef ASSEMBLER_HPP
#define ASSEMBLER_HPP

#include <cstdint>
#include <cstdio>
#include <memory>
#include <vector>
#include <string>
#include <map>

/* Maximum line length for assembly source */
#define MAX_LINE 512

/*
 * Custom deleter for FILE* to use with smart pointers
 */
struct FileDeleter {
	void operator()(FILE* f) const {
		if (f) {
			fclose(f);
		}
	}
};

/* Type alias for unique_ptr with FILE* */
using FilePtr = std::unique_ptr<FILE, FileDeleter>;

/*
 * Assembly section types
 *
 * SEC_TEXT: Executable code section
 * SEC_DATA: Data section (initialized data)
 * SEC_RODATA: Read-only data section
 * SEC_BSS: Uninitialized data section
 * SEC_CUSTOM: Custom section (name stored separately)
 */
enum SectionType {
	SEC_TEXT,
	SEC_DATA,
	SEC_RODATA,
	SEC_BSS,
	SEC_CUSTOM
};

/*
 * Section information structure
 *
 * name: Section name (e.g., ".text", ".data", ".rodata", ".bss", or custom)
 * type: Section type (for standard sections)
 * offset: Current offset/size in this section
 * base_addr: Base address assigned to section (set during adjust phase)
 */
struct SectionInfo {
	std::string name;
	SectionType type;
	uint32_t offset;
	uint32_t base_addr;

	SectionInfo() : name(".text"), type(SEC_TEXT), offset(0), base_addr(0) {}
	SectionInfo(const std::string& n, SectionType t) : name(n), type(t), offset(0), base_addr(0) {}
};

/* Backwards compatibility alias */
typedef SectionType Section;

/*
 * Label definition structure
 *
 * name: Label identifier
 * addr: Absolute address assigned to label
 * section_name: Name of section where label is defined
 */
struct Label {
	std::string name;
	uint32_t addr;
	std::string section_name;
};

/*
 * Static encoder class for RISC-V instruction encoding
 */
class Encoder {
public:
	/**
	 * Encode R-type (register) instruction
	 *
	 * funct7: 7-bit function code (upper bits)
	 * rs2: Source register 2 (5 bits)
	 * rs1: Source register 1 (5 bits)
	 * funct3: 3-bit function code
	 * rd: Destination register (5 bits)
	 * opcode: 7-bit opcode
	 *
	 * Output: Encoded 32-bit instruction
	 */
	static uint32_t encode_r(uint32_t funct7, uint32_t rs2, uint32_t rs1,
			uint32_t funct3, uint32_t rd, uint32_t opcode);

	/**
	 * Encode I-type (immediate) instruction
	 *
	 * imm: 12-bit immediate value (sign-extended)
	 * rs1: Source register 1 (5 bits)
	 * funct3: 3-bit function code
	 * rd: Destination register (5 bits)
	 * opcode: 7-bit opcode
	 *
	 * Output: Encoded 32-bit instruction
	 */
	static uint32_t encode_i(int32_t imm, uint32_t rs1,
			uint32_t funct3, uint32_t rd, uint32_t opcode);

	/**
	 * Encode S-type (store) instruction
	 *
	 * imm: 12-bit immediate value (split into two fields)
	 * rs2: Source register 2 (5 bits)
	 * rs1: Source register 1 (5 bits)
	 * funct3: 3-bit function code
	 * opcode: 7-bit opcode
	 *
	 * Output: Encoded 32-bit instruction
	 */
	static uint32_t encode_s(int32_t imm, uint32_t rs2, uint32_t rs1,
			uint32_t funct3, uint32_t opcode);

	/**
	 * Encode B-type (branch) instruction
	 *
	 * imm: 13-bit immediate value (word-aligned, split into multiple fields)
	 * rs2: Source register 2 (5 bits)
	 * rs1: Source register 1 (5 bits)
	 * funct3: 3-bit function code
	 * opcode: 7-bit opcode
	 *
	 * Output: Encoded 32-bit instruction
	 */
	static uint32_t encode_b(int32_t imm, uint32_t rs2, uint32_t rs1,
			uint32_t funct3, uint32_t opcode);

	/**
	 * Encode U-type (upper immediate) instruction
	 *
	 * imm: 20-bit immediate value (aligned to 12-bit boundary)
	 * rd: Destination register (5 bits)
	 * opcode: 7-bit opcode
	 *
	 * Output: Encoded 32-bit instruction
	 */
	static uint32_t encode_u(int32_t imm, uint32_t rd, uint32_t opcode);

	/**
	 * Encode J-type (jump) instruction
	 *
	 * imm: 21-bit immediate value (word-aligned, split into multiple fields)
	 * rd: Destination register (5 bits)
	 * opcode: 7-bit opcode
	 *
	 * Output: Encoded 32-bit instruction
	 */
	static uint32_t encode_j(int32_t imm, uint32_t rd, uint32_t opcode);
};

/*
 * Main assembler class
 */
class Assembler {
private:
	std::vector<Label> labels;
	std::map<std::string, SectionInfo> sections;
	std::string current_section_name;
	uint32_t pc_text;  /* Kept for backwards compatibility */
	uint32_t pc_data;  /* Kept for backwards compatibility */
	uint32_t text_size;
	uint32_t data_size;

	/* Utility functions (private instance methods) */
	uint32_t find_label(const char *name) const;
	int32_t parse_imm(const char *s) const;
	void switch_section(const std::string& section_name);
	SectionInfo& get_current_section();
	const SectionInfo& get_current_section() const;
	SectionType get_section_type(const std::string& name) const;

	/* First pass helpers */
	int pseudoinstruction_size(const char *op, const char *a2) const;
	void parse_simple_args(const char *s, char *op, char *a1, char *a2) const;
	void process_instruction_first_pass(const char *s);
	void process_label(char *s);
	void process_directive(char *s);

	/* Second pass helpers */
	uint32_t encode_instruction(uint32_t current_pc, const char *op,
			const char *a1, const char *a2, const char *a3) const;
	void parse_instruction_args(const char *s, char *op, char *a1,
			char *a2, char *a3) const;
	void process_data_directive(FILE *out, char *s, uint32_t *pc) const;
	void process_instruction_second_pass(FILE *out, uint32_t *pc, const char *s) const;

	/* Pseudoinstruction expansion */
	int expand_pseudoinstruction(const char *op, const char *a1, const char *a2,
			char out_lines[2][MAX_LINE], uint32_t current_pc) const;

public:
	/**
	 * Static utility functions (exposed for testing)
	 */
	static char *trim(char *s);
	static int reg_num(const char *r);
	static size_t parse_escaped_string(const char *src, uint8_t *out);

	/**
	 * Constructor
	 */
	Assembler();

	/**
	 * First assembly pass: label resolution and address calculation
	 *
	 * f: Input assembly file stream
	 *
	 * Performs:
	 * Section tracking (.text, .data)
	 * Label address assignment (relative to section start)
	 * Size calculation for text and data sections
	 */
	void first_pass(FILE *f);

	/**
	 * Adjust label addresses after first pass
	 *
	 * data_base: Base address for data section (typically end of text section)
	 *
	 * Performs:
	 * Converts label addresses from section-relative to absolute
	 * Text labels get their relative address
	 * Data labels get data_base + their relative address
	 */
	void adjust_labels(uint32_t data_base);

	/**
	 * Second assembly pass: instruction encoding and output generation
	 *
	 * in: Input assembly file stream (rewound after first pass)
	 * out: Output binary file stream
	 *
	 * Performs:
	 * Instruction parsing and encoding
	 * Data directive processing (.ascii, .byte)
	 * Binary output to appropriate file positions
	 * Label reference resolution using adjusted addresses
	 */
	void second_pass(FILE *in, FILE *out);

	/**
	 * Get text section size
	 */
	uint32_t get_text_size() const { return text_size; }

	/**
	 * Get data section size
	 */
	uint32_t get_data_size() const { return data_size; }

	/**
	 * Get number of labels
	 */
	size_t get_label_count() const { return labels.size(); }
};

#endif