/* expand_pseudoinstructions.c */
#include "assembler.hpp"
#include <cstring>
#include <cstdlib>
#include <cstdio>

int expand_pseudoinstruction(const char *op, const char *a1, const char *a2,
		const assembler_state_t *state, char out_lines[2][MAX_LINE], uint32_t current_pc) {
	if (!strcmp(op, "nop")) {
		snprintf(out_lines[0], MAX_LINE, "addi x0, x0, 0");
		return 1;
	}

	if (!strcmp(op, "mv")) {
		snprintf(out_lines[0], MAX_LINE, "addi %s, %s, 0", a1, a2);
		return 1;
	}

	if (!strcmp(op, "li")) {
		int32_t imm = parse_imm(state, a2);

		/* Check if immediate fits in 12 bits */
		if (imm >= -2048 && imm <= 2047) {
			/* Single addi instruction */
			snprintf(out_lines[0], MAX_LINE, "addi %s, x0, %d", a1, imm);
			return 1;
		} else {
			/* Two instructions: lui + addi */
			int32_t hi20 = (imm + 0x800) >> 12;
			int32_t lo12 = imm - (hi20 << 12);

			snprintf(out_lines[0], MAX_LINE, "lui %s, %d", a1, hi20);
			snprintf(out_lines[1], MAX_LINE, "addi %s, %s, %d", a1, a1, lo12);
			return 2;
		}
	}

	if (!strcmp(op, "la")) {
		uint32_t target_addr = find_label(state, a2);

		/* Calculate PC-relative offset */
		int32_t offset = target_addr - current_pc;
		int32_t hi20 = (offset + 0x800) >> 12;
		int32_t lo12 = offset - (hi20 << 12);

		/* auipc + addi */
		snprintf(out_lines[0], MAX_LINE, "auipc %s, %d", a1, hi20);
		snprintf(out_lines[1], MAX_LINE, "addi %s, %s, %d", a1, a1, lo12);
		return 2;
	}

	/* Not a pseudoinstruction */
	return 0;
}