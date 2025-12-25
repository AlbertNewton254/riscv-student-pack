// second_pass.c
#include "assembler.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

static uint32_t encode_instruction(const assembler_state_t *state, uint32_t current_pc,
								   const char *op, const char *a1,
								   const char *a2, const char *a3) {
	if (!strcmp(op, "add")) {
		return encode_r(0x00, reg_num(a3), reg_num(a2), 0x0, reg_num(a1), 0x33);
	} else if (!strcmp(op, "sub")) {
		return encode_r(0x20, reg_num(a3), reg_num(a2), 0x0, reg_num(a1), 0x33);
	} else if (!strcmp(op, "sll")) {
		return encode_r(0x00, reg_num(a3), reg_num(a2), 0x1, reg_num(a1), 0x33);
	} else if (!strcmp(op, "slt")) {
		return encode_r(0x00, reg_num(a3), reg_num(a2), 0x2, reg_num(a1), 0x33);
	} else if (!strcmp(op, "sltu")) {
		return encode_r(0x00, reg_num(a3), reg_num(a2), 0x3, reg_num(a1), 0x33);
	} else if (!strcmp(op, "xor")) {
		return encode_r(0x00, reg_num(a3), reg_num(a2), 0x4, reg_num(a1), 0x33);
	} else if (!strcmp(op, "srl")) {
		return encode_r(0x00, reg_num(a3), reg_num(a2), 0x5, reg_num(a1), 0x33);
	} else if (!strcmp(op, "sra")) {
		return encode_r(0x20, reg_num(a3), reg_num(a2), 0x5, reg_num(a1), 0x33);
	} else if (!strcmp(op, "or")) {
		return encode_r(0x00, reg_num(a3), reg_num(a2), 0x6, reg_num(a1), 0x33);
	} else if (!strcmp(op, "and")) {
		return encode_r(0x00, reg_num(a3), reg_num(a2), 0x7, reg_num(a1), 0x33);
	} else if (!strcmp(op, "addi")) {
		return encode_i(parse_imm(state, a3), reg_num(a2), 0x0, reg_num(a1), 0x13);
	} else if (!strcmp(op, "slti")) {
		return encode_i(parse_imm(state, a3), reg_num(a2), 0x2, reg_num(a1), 0x13);
	} else if (!strcmp(op, "sltiu")) {
		return encode_i(parse_imm(state, a3), reg_num(a2), 0x3, reg_num(a1), 0x13);
	} else if (!strcmp(op, "xori")) {
		return encode_i(parse_imm(state, a3), reg_num(a2), 0x4, reg_num(a1), 0x13);
	} else if (!strcmp(op, "ori")) {
		return encode_i(parse_imm(state, a3), reg_num(a2), 0x6, reg_num(a1), 0x13);
	} else if (!strcmp(op, "andi")) {
		return encode_i(parse_imm(state, a3), reg_num(a2), 0x7, reg_num(a1), 0x13);
	} else if (!strcmp(op, "slli")) {
		return encode_r(0x00, parse_imm(state, a3) & 0x1F, reg_num(a2), 0x1, reg_num(a1), 0x13);
	} else if (!strcmp(op, "srli")) {
		return encode_r(0x00, parse_imm(state, a3) & 0x1F, reg_num(a2), 0x5, reg_num(a1), 0x13);
	} else if (!strcmp(op, "srai")) {
		return encode_r(0x20, parse_imm(state, a3) & 0x1F, reg_num(a2), 0x5, reg_num(a1), 0x13);
	} else if (!strcmp(op, "lb")) {
		return encode_i(parse_imm(state, a3), reg_num(a2), 0x0, reg_num(a1), 0x03);
	} else if (!strcmp(op, "lh")) {
		return encode_i(parse_imm(state, a3), reg_num(a2), 0x1, reg_num(a1), 0x03);
	} else if (!strcmp(op, "lw")) {
		return encode_i(parse_imm(state, a3), reg_num(a2), 0x2, reg_num(a1), 0x03);
	} else if (!strcmp(op, "lbu")) {
		return encode_i(parse_imm(state, a3), reg_num(a2), 0x4, reg_num(a1), 0x03);
	} else if (!strcmp(op, "lhu")) {
		return encode_i(parse_imm(state, a3), reg_num(a2), 0x5, reg_num(a1), 0x03);
	} else if (!strcmp(op, "sb")) {
		return encode_s(parse_imm(state, a3), reg_num(a1), reg_num(a2), 0x0, 0x23);
	} else if (!strcmp(op, "sh")) {
		return encode_s(parse_imm(state, a3), reg_num(a1), reg_num(a2), 0x1, 0x23);
	} else if (!strcmp(op, "sw")) {
		return encode_s(parse_imm(state, a3), reg_num(a1), reg_num(a2), 0x2, 0x23);
	} else if (!strcmp(op, "beq")) {
		int32_t target = parse_imm(state, a3);
		int32_t offset = target - current_pc;
		return encode_b(offset, reg_num(a2), reg_num(a1), 0x0, 0x63);
	} else if (!strcmp(op, "bne")) {
		int32_t target = parse_imm(state, a3);
		int32_t offset = target - current_pc;
		return encode_b(offset, reg_num(a2), reg_num(a1), 0x1, 0x63);
	} else if (!strcmp(op, "blt")) {
		int32_t target = parse_imm(state, a3);
		int32_t offset = target - current_pc;
		return encode_b(offset, reg_num(a2), reg_num(a1), 0x4, 0x63);
	} else if (!strcmp(op, "bge")) {
		int32_t target = parse_imm(state, a3);
		int32_t offset = target - current_pc;
		return encode_b(offset, reg_num(a2), reg_num(a1), 0x5, 0x63);
	} else if (!strcmp(op, "bltu")) {
		int32_t target = parse_imm(state, a3);
		int32_t offset = target - current_pc;
		return encode_b(offset, reg_num(a2), reg_num(a1), 0x6, 0x63);
	} else if (!strcmp(op, "bgeu")) {
		int32_t target = parse_imm(state, a3);
		int32_t offset = target - current_pc;
		return encode_b(offset, reg_num(a2), reg_num(a1), 0x7, 0x63);
	} else if (!strcmp(op, "jal")) {
		int32_t target = parse_imm(state, a2);
		int32_t offset = target - current_pc;
		return encode_j(offset, reg_num(a1), 0x6F);
	} else if (!strcmp(op, "jalr")) {
		return encode_i(parse_imm(state, a3), reg_num(a2), 0x0, reg_num(a1), 0x67);
	} else if (!strcmp(op, "lui")) {
		return encode_u(parse_imm(state, a2), reg_num(a1), 0x37);
	} else if (!strcmp(op, "auipc")) {
		return encode_u(parse_imm(state, a2), reg_num(a1), 0x17);
	} else if (!strcmp(op, "ecall")) {
		return encode_i(0x000, 0x00, 0x0, 0x00, 0x73);
	} else if (!strcmp(op, "ebreak")) {
		return encode_i(0x001, 0x00, 0x0, 0x00, 0x73);
	}

	fprintf(stderr, "Unknown instruction: %s\n", op);
	exit(1);
}

static void parse_instruction_args(const char *s, char *op, char *a1, char *a2, char *a3) {
	char *args = strchr(s, ' ');
	if (!args) {
		strcpy(op, s);
		return;
	}

	strncpy(op, s, args - s);
	op[args - s] = '\0';

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

		char *temp = between;
		while (*temp && *temp != ',') temp++;

		strncpy(a2, between, temp - between);
		a2[temp - between] = '\0';

		strcpy(a3, trim(comma2 + 1));
	}
}

static void process_data_directive(FILE *out, char *s, uint32_t *pc) {
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

	} else if (!strncmp(s, ".word", 5)) {
		char *ptr = s + 5;
		while (*ptr) {
			if (*ptr != ' ' && *ptr != '\t' && *ptr != ',') {
				int32_t val = parse_imm(NULL, ptr);
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

void second_pass(FILE *in, FILE *out, const assembler_state_t *state) {
	char line[MAX_LINE];
	section_t current_section = SEC_TEXT;
	uint32_t pc = 0;
	uint32_t data_base = state->pc_text;

	rewind(in);

	while (fgets(line, sizeof(line), in)) {
		char *s = trim(line);
		if (*s == 0 || *s == '#') continue;

		if (!strcmp(s, ".text")) {
			current_section = SEC_TEXT;
			pc = 0;
			fseek(out, 0, SEEK_SET);
			continue;
		}

		if (!strcmp(s, ".data")) {
			current_section = SEC_DATA;
			pc = data_base;
			fseek(out, data_base, SEEK_SET);
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
			if (current_section == SEC_DATA) {
				process_data_directive(out, s, &pc);
			}
			continue;
		}

		if (current_section == SEC_TEXT) {
			char op[16] = "", a1[32] = "", a2[32] = "", a3[32] = "";
			parse_instruction_args(s, op, a1, a2, a3);

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

			uint32_t instr = encode_instruction(state, pc, op, a1, a2, a3);
			fwrite(&instr, 4, 1, out);
			pc += 4;
		}
	}
}
