/* second_pass.cpp */
#include "assembler.hpp"
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <cstdarg>

static void safe_snprintf(char *dest, size_t size, const char *format, ...) {
	va_list args;
	va_start(args, format);
	vsnprintf(dest, size, format, args);
	va_end(args);
	dest[size - 1] = '\0';
}

uint32_t Assembler::encode_instruction(uint32_t current_pc, const char *op, const char *a1, const char *a2, const char *a3) const {
	if (!strcmp(op, "add")) {
		return Encoder::encode_r(0x00, reg_num(a3), reg_num(a2), 0x0, reg_num(a1), 0x33);
	} else if (!strcmp(op, "sub")) {
		return Encoder::encode_r(0x20, reg_num(a3), reg_num(a2), 0x0, reg_num(a1), 0x33);
	} else if (!strcmp(op, "sll")) {
		return Encoder::encode_r(0x00, reg_num(a3), reg_num(a2), 0x1, reg_num(a1), 0x33);
	} else if (!strcmp(op, "slt")) {
		return Encoder::encode_r(0x00, reg_num(a3), reg_num(a2), 0x2, reg_num(a1), 0x33);
	} else if (!strcmp(op, "sltu")) {
		return Encoder::encode_r(0x00, reg_num(a3), reg_num(a2), 0x3, reg_num(a1), 0x33);
	} else if (!strcmp(op, "xor")) {
		return Encoder::encode_r(0x00, reg_num(a3), reg_num(a2), 0x4, reg_num(a1), 0x33);
	} else if (!strcmp(op, "srl")) {
		return Encoder::encode_r(0x00, reg_num(a3), reg_num(a2), 0x5, reg_num(a1), 0x33);
	} else if (!strcmp(op, "sra")) {
		return Encoder::encode_r(0x20, reg_num(a3), reg_num(a2), 0x5, reg_num(a1), 0x33);
	} else if (!strcmp(op, "or")) {
		return Encoder::encode_r(0x00, reg_num(a3), reg_num(a2), 0x6, reg_num(a1), 0x33);
	} else if (!strcmp(op, "and")) {
		return Encoder::encode_r(0x00, reg_num(a3), reg_num(a2), 0x7, reg_num(a1), 0x33);
	} else if (!strcmp(op, "addi")) {
		return Encoder::encode_i(parse_imm(a3), reg_num(a2), 0x0, reg_num(a1), 0x13);
	} else if (!strcmp(op, "slti")) {
		return Encoder::encode_i(parse_imm(a3), reg_num(a2), 0x2, reg_num(a1), 0x13);
	} else if (!strcmp(op, "sltiu")) {
		return Encoder::encode_i(parse_imm(a3), reg_num(a2), 0x3, reg_num(a1), 0x13);
	} else if (!strcmp(op, "xori")) {
		return Encoder::encode_i(parse_imm(a3), reg_num(a2), 0x4, reg_num(a1), 0x13);
	} else if (!strcmp(op, "ori")) {
		return Encoder::encode_i(parse_imm(a3), reg_num(a2), 0x6, reg_num(a1), 0x13);
	} else if (!strcmp(op, "andi")) {
		return Encoder::encode_i(parse_imm(a3), reg_num(a2), 0x7, reg_num(a1), 0x13);
	} else if (!strcmp(op, "slli")) {
		return Encoder::encode_r(0x00, parse_imm(a3) & 0x1F, reg_num(a2), 0x1, reg_num(a1), 0x13);
	} else if (!strcmp(op, "srli")) {
		return Encoder::encode_r(0x00, parse_imm(a3) & 0x1F, reg_num(a2), 0x5, reg_num(a1), 0x13);
	} else if (!strcmp(op, "srai")) {
		return Encoder::encode_r(0x20, parse_imm(a3) & 0x1F, reg_num(a2), 0x5, reg_num(a1), 0x13);
	} else if (!strcmp(op, "lb")) {
		return Encoder::encode_i(parse_imm(a2), reg_num(a3), 0x0, reg_num(a1), 0x03);
	} else if (!strcmp(op, "lh")) {
		return Encoder::encode_i(parse_imm(a2), reg_num(a3), 0x1, reg_num(a1), 0x03);
	} else if (!strcmp(op, "lw")) {
		return Encoder::encode_i(parse_imm(a2), reg_num(a3), 0x2, reg_num(a1), 0x03);
	} else if (!strcmp(op, "lbu")) {
		return Encoder::encode_i(parse_imm(a2), reg_num(a3), 0x4, reg_num(a1), 0x03);
	} else if (!strcmp(op, "lhu")) {
		return Encoder::encode_i(parse_imm(a2), reg_num(a3), 0x5, reg_num(a1), 0x03);
	} else if (!strcmp(op, "sb")) {
		return Encoder::encode_s(parse_imm(a2), reg_num(a1), reg_num(a3), 0x0, 0x23);
	} else if (!strcmp(op, "sh")) {
		return Encoder::encode_s(parse_imm(a2), reg_num(a1), reg_num(a3), 0x1, 0x23);
	} else if (!strcmp(op, "sw")) {
		return Encoder::encode_s(parse_imm(a2), reg_num(a1), reg_num(a3), 0x2, 0x23);
	} else if (!strcmp(op, "beq")) {
		int32_t target = parse_imm(a3);
		int32_t offset = target - current_pc;
		return Encoder::encode_b(offset, reg_num(a2), reg_num(a1), 0x0, 0x63);
	} else if (!strcmp(op, "bne")) {
		int32_t target = parse_imm(a3);
		int32_t offset = target - current_pc;
		return Encoder::encode_b(offset, reg_num(a2), reg_num(a1), 0x1, 0x63);
	} else if (!strcmp(op, "blt")) {
		int32_t target = parse_imm(a3);
		int32_t offset = target - current_pc;
		return Encoder::encode_b(offset, reg_num(a2), reg_num(a1), 0x4, 0x63);
	} else if (!strcmp(op, "bge")) {
		int32_t target = parse_imm(a3);
		int32_t offset = target - current_pc;
		return Encoder::encode_b(offset, reg_num(a2), reg_num(a1), 0x5, 0x63);
	} else if (!strcmp(op, "bltu")) {
		int32_t target = parse_imm(a3);
		int32_t offset = target - current_pc;
		return Encoder::encode_b(offset, reg_num(a2), reg_num(a1), 0x6, 0x63);
	} else if (!strcmp(op, "bgeu")) {
		int32_t target = parse_imm(a3);
		int32_t offset = target - current_pc;
		return Encoder::encode_b(offset, reg_num(a2), reg_num(a1), 0x7, 0x63);
	} else if (!strcmp(op, "jal")) {
		int32_t target = parse_imm(a2);
		int32_t offset = target - current_pc;
		return Encoder::encode_j(offset, reg_num(a1), 0x6F);
	} else if (!strcmp(op, "jalr")) {
		return Encoder::encode_i(parse_imm(a3), reg_num(a2), 0x0, reg_num(a1), 0x67);
	} else if (!strcmp(op, "lui")) {
		return Encoder::encode_u(parse_imm(a2), reg_num(a1), 0x37);
	} else if (!strcmp(op, "auipc")) {
		return Encoder::encode_u(parse_imm(a2), reg_num(a1), 0x17);
	} else if (!strcmp(op, "ecall")) {
		return Encoder::encode_i(0x000, 0x00, 0x0, 0x00, 0x73);
	} else if (!strcmp(op, "ebreak")) {
		return Encoder::encode_i(0x001, 0x00, 0x0, 0x00, 0x73);
	}

	fprintf(stderr, "Unknown instruction: %s\n", op);
	exit(1);
}

void Assembler::parse_instruction_args(const char *s, char *op, char *a1, char *a2, char *a3) const {
	char temp[MAX_LINE];
	strncpy(temp, s, MAX_LINE - 1);
	temp[MAX_LINE - 1] = '\0';

	char *args = strchr(temp, ' ');
	if (!args) {
		strcpy(op, temp);
		return;
	}

	strncpy(op, temp, args - temp);
	op[args - temp] = '\0';

	args = trim(args);

	char *comma1 = strchr(args, ',');
	char *comma2 = comma1 ? strchr(comma1 + 1, ',') : NULL;

	if (!comma1) {
		strcpy(a1, args);
	} else if (!comma2) {
		strncpy(a1, args, comma1 - args);
		a1[comma1 - args] = '\0';
		strcpy(a2, trim(comma1 + 1));
	} else {
		strncpy(a1, args, comma1 - args);
		a1[comma1 - args] = '\0';

		char *between = comma1 + 1;
		while (isspace(*between)) between++;

		char *temp_ptr = between;
		while (*temp_ptr && *temp_ptr != ',') temp_ptr++;

		strncpy(a2, between, temp_ptr - between);
		a2[temp_ptr - between] = '\0';

		strcpy(a3, trim(comma2 + 1));
	}
}

void Assembler::process_data_directive(FILE *out, char *s, uint32_t *pc) const {
	s = trim(s);

	if (!strncmp(s, ".ascii", 6)) {
		char *q = strchr(s, '"');
		if (!q) {
			fprintf(stderr, "Malformed .ascii\n");
			exit(1);
		}
		uint8_t buf[1024];
		size_t n = parse_escaped_string(q + 1, buf);
		fwrite(buf, 1, n, out);
		(*pc) += n;

	} else if (!strncmp(s, ".asciiz", 7)) {
		char *q = strchr(s, '"');
		if (!q) {
			fprintf(stderr, "Malformed .asciiz\n");
			exit(1);
		}
		uint8_t buf[1024];
		size_t n = parse_escaped_string(q + 1, buf);
		fwrite(buf, 1, n, out);
		uint8_t zero = 0;
		fwrite(&zero, 1, 1, out);
		(*pc) += n + 1;

	} else if (!strncmp(s, ".byte", 5)) {
		char *ptr = s + 5;
		while (*ptr) {
			if (*ptr != ' ' && *ptr != '\t' && *ptr != ',') {
				int val = strtol(ptr, &ptr, 0);
				uint8_t b = val & 0xFF;
				fwrite(&b, 1, 1, out);
				(*pc)++;
			}
			while (*ptr == ',' || isspace(*ptr)) ptr++;
		}

	} else if (!strncmp(s, ".half", 5)) {
		char *ptr = s + 5;
		while (*ptr) {
			if (*ptr != ' ' && *ptr != '\t' && *ptr != ',') {
				int32_t val = parse_imm(ptr);
				ptr += strspn(ptr, "0123456789abcdefABCDEFxX-+");
				uint16_t h = val & 0xFFFF;
				fwrite(&h, 2, 1, out);
				(*pc) += 2;
			}
			while (*ptr == ',' || isspace(*ptr)) ptr++;
		}

	} else if (!strncmp(s, ".word", 5)) {
		char *ptr = s + 5;
		while (*ptr) {
			if (*ptr != ' ' && *ptr != '\t' && *ptr != ',') {
				int32_t val = parse_imm(ptr);
				ptr += strspn(ptr, "0123456789abcdefABCDEFxX-+");
				fwrite(&val, 4, 1, out);
				(*pc) += 4;
			}
			while (*ptr == ',' || isspace(*ptr)) ptr++;
		}

	} else if (!strncmp(s, ".space", 6)) {
		int size;
		if (sscanf(s + 6, "%d", &size) == 1) {
			for (int i = 0; i < size; i++) {
				uint8_t zero = 0;
				fwrite(&zero, 1, 1, out);
				(*pc)++;
			}
		}
	}
}

static int is_pseudoinstruction(const char *op) {
	return !strcmp(op, "li") || !strcmp(op, "la") ||
		   !strcmp(op, "mv") || !strcmp(op, "nop");
}

static void debug_parsing(const char *original_line, const char *op, const char *a1, const char *a2, const char *a3) {
	fprintf(stderr, "DEBUG PARSING:\n");
	fprintf(stderr, "\tOriginal line: '%s'\n", original_line);
	fprintf(stderr, "\tParsed:\n");
	fprintf(stderr, "\t\top:  '%s'\n", op);
	fprintf(stderr, "\t\ta1:  '%s'\n", a1);
	fprintf(stderr, "\t\ta2:  '%s'\n", a2);
	fprintf(stderr, "\t\ta3:  '%s'\n", a3);
}

void Assembler::process_instruction_second_pass(FILE *out, uint32_t *pc, const char *s) const {
	char original_line[MAX_LINE];
	strncpy(original_line, s, MAX_LINE - 1);
	original_line[MAX_LINE - 1] = '\0';

	char line_copy[MAX_LINE];
	strncpy(line_copy, s, MAX_LINE - 1);
	line_copy[MAX_LINE - 1] = '\0';

	char *comment = strchr(line_copy, '#');
	if (comment) {
		*comment = '\0';
	}

	char *line_ptr = trim(line_copy);

	char processed_line[MAX_LINE];
	strcpy(processed_line, line_ptr);

	char *open_paren = strchr(processed_line, '(');
	if (open_paren) {
		char *close_paren = strchr(open_paren, ')');
		if (close_paren) {
			char *offset_start = open_paren;
			while (offset_start > processed_line && !isspace(*(offset_start-1)) && *(offset_start-1) != ',') {
				offset_start--;
			}

			char offset[32] = "";
			strncpy(offset, offset_start, open_paren - offset_start);
			offset[open_paren - offset_start] = '\0';

			char reg[32] = "";
			strncpy(reg, open_paren + 1, close_paren - (open_paren + 1));
			reg[close_paren - (open_paren + 1)] = '\0';

			char temp[MAX_LINE];

			strncpy(temp, processed_line, offset_start - processed_line);
			temp[offset_start - processed_line] = '\0';

			char suffix[MAX_LINE];
			strcpy(suffix, close_paren + 1);

			safe_snprintf(processed_line, MAX_LINE, "%s%s,%s%s",
					temp, offset, reg, suffix);
		}
	}

	char op[16] = "", a1[32] = "", a2[32] = "", a3[32] = "";

	fprintf(stderr, "\tProcessed line: '%s'\n", processed_line);

	parse_instruction_args(processed_line, op, a1, a2, a3);

	trim(a1);
	trim(a2);
	trim(a3);

	if (a1[0] && a1[strlen(a1)-1] == ',') {
		a1[strlen(a1)-1] = '\0';
		trim(a1);
	}
	if (a2[0] && a2[strlen(a2)-1] == ',') {
		a2[strlen(a2)-1] = '\0';
		trim(a2);
	}

	debug_parsing(original_line, op, a1, a2, a3);

	if (is_pseudoinstruction(op)) {
		char expanded[2][MAX_LINE];
		int count = expand_pseudoinstruction(op, a1, a2, expanded, *pc);

		fprintf(stderr, "\tPseudo-instruction expanded to %d instruction(s):\n", count);
		for (int i = 0; i < count; i++) {
			fprintf(stderr, "\t\t[%d] %s\n", i, expanded[i]);
		}

		for (int i = 0; i < count; i++) {
			char exp_op[16], exp_a1[32], exp_a2[32], exp_a3[32];

			char exp_line[MAX_LINE];
			strcpy(exp_line, expanded[i]);

			comment = strchr(exp_line, '#');
			if (comment) {
				*comment = '\0';
			}

			parse_instruction_args(exp_line, exp_op, exp_a1, exp_a2, exp_a3);

			trim(exp_a1);
			trim(exp_a2);
			trim(exp_a3);

			fprintf(stderr, "\tParsing expanded instruction %d:\n", i);
			fprintf(stderr, "\t\top:  '%s'\n", exp_op);
			fprintf(stderr, "\t\ta1:  '%s'\n", exp_a1);
			fprintf(stderr, "\t\ta2:  '%s'\n", exp_a2);
			fprintf(stderr, "\t\ta3:  '%s'\n", exp_a3);

			uint32_t instr = encode_instruction(*pc, exp_op, exp_a1, exp_a2, exp_a3);
			fwrite(&instr, 4, 1, out);
			*pc += 4;
		}
	} else {
		uint32_t instr = encode_instruction(*pc, op, a1, a2, a3);
		fwrite(&instr, 4, 1, out);
		*pc += 4;
	}
}

void Assembler::second_pass(FILE *in, FILE *out) {
	char line[MAX_LINE];
	std::string current_section_name = ".text";
	uint32_t pc = 0;
	uint32_t data_base = pc_text;

	rewind(in);

	while (fgets(line, sizeof(line), in)) {
		char *s = trim(line);
		if (*s == 0 || *s == '#') continue;

		/* Handle .section directive */
		if (!strncmp(s, ".section", 8)) {
			char *section_start = s + 8;
			while (*section_start && isspace(*section_start)) section_start++;

			char section_name[64] = "";
			int i = 0;
			while (section_start[i] && !isspace(section_start[i]) &&
			       section_start[i] != ',' && i < 63) {
				section_name[i] = section_start[i];
				i++;
			}
			section_name[i] = '\0';

			if (section_name[0]) {
				current_section_name = section_name;
				auto it = sections.find(current_section_name);
				if (it != sections.end()) {
					pc = it->second.base_addr;
					fseek(out, pc, SEEK_SET);
				}
			}
			continue;
		}

		/* Handle legacy directives */
		if (!strcmp(s, ".text")) {
			current_section_name = ".text";
			pc = 0;
			fseek(out, 0, SEEK_SET);
			continue;
		}

		if (!strcmp(s, ".data")) {
			current_section_name = ".data";
			pc = data_base;
			fseek(out, data_base, SEEK_SET);
			continue;
		}

		if (!strcmp(s, ".rodata")) {
			current_section_name = ".rodata";
			auto it = sections.find(".rodata");
			if (it != sections.end()) {
				pc = it->second.base_addr;
				fseek(out, pc, SEEK_SET);
			}
			continue;
		}

		if (!strcmp(s, ".bss")) {
			current_section_name = ".bss";
			auto it = sections.find(".bss");
			if (it != sections.end()) {
				pc = it->second.base_addr;
				fseek(out, pc, SEEK_SET);
			}
			continue;
		}

		if (strstr(s, ".globl")) {
			continue;
		}

		char *colon = strchr(s, ':');
		if (colon) {
			char *after_colon = colon + 1;
			while (*after_colon && isspace(*after_colon)) {
				after_colon++;
			}
			if (*after_colon && *after_colon != '#') {
				s = trim(after_colon);
			} else {
				continue;
			}
		}

		if (*s == '.') {
			SectionType sec_type = SEC_DATA;
			auto it = sections.find(current_section_name);
			if (it != sections.end()) {
				sec_type = it->second.type;
			}
			if (sec_type != SEC_TEXT) {
				process_data_directive(out, s, &pc);
			}
			continue;
		}

		SectionType sec_type = SEC_TEXT;
		auto it = sections.find(current_section_name);
		if (it != sections.end()) {
			sec_type = it->second.type;
		}
		if (sec_type == SEC_TEXT) {
			process_instruction_second_pass(out, &pc, s);
		}
	}
}