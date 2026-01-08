/* expand_pseudoinstruction.cpp */
#include "assembler.hpp"
#include <cstring>
#include <cstdlib>
#include <cstdio>

int Assembler::expand_pseudoinstruction(const char *op, const char *a1, const char *a2,
		char out_lines[2][MAX_LINE], uint32_t current_pc) const {
	if (!strcmp(op, "nop")) {
		snprintf(out_lines[0], MAX_LINE, "addi x0, x0, 0");
		return 1;
	}

	if (!strcmp(op, "mv")) {
		snprintf(out_lines[0], MAX_LINE, "addi %s, %s, 0", a1, a2);
		return 1;
	}

	if (!strcmp(op, "li")) {
		int32_t imm = parse_imm(a2);

		if (imm >= -2048 && imm <= 2047) {
			snprintf(out_lines[0], MAX_LINE, "addi %s, x0, %d", a1, imm);
			return 1;
		} else {
			int32_t hi20 = (imm + 0x800) >> 12;
			int32_t lo12 = imm - (hi20 << 12);

			snprintf(out_lines[0], MAX_LINE, "lui %s, %d", a1, hi20);
			snprintf(out_lines[1], MAX_LINE, "addi %s, %s, %d", a1, a1, lo12);
			return 2;
		}
	}

	if (!strcmp(op, "la")) {
		uint32_t target_addr = find_label(a2);

		int32_t offset = target_addr - current_pc;
		int32_t hi20 = (offset + 0x800) >> 12;
		int32_t lo12 = offset - (hi20 << 12);

		snprintf(out_lines[0], MAX_LINE, "auipc %s, %d", a1, hi20);
		snprintf(out_lines[1], MAX_LINE, "addi %s, %s, %d", a1, a1, lo12);
		return 2;
	}

	if (!strcmp(op, "call")) {
		snprintf(out_lines[0], MAX_LINE, "jal x1, %s", a1);
		return 1;
	}

	if (!strcmp(op, "ret")) {
		snprintf(out_lines[0], MAX_LINE, "jalr x0, x1, 0");
		return 1;
	}

	if (!strcmp(op, "j")) {
		snprintf(out_lines[0], MAX_LINE, "jal x0, %s", a1);
		return 1;
	}

	return 0;
}