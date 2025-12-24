#include "assembler.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

static void process_label(assembler_state_t *state, char *s) {
	char *colon = strchr(s, ':');
	if (!colon) return;

	*colon = 0;
	strcpy(state->labels[state->label_count].name, trim(s));
	state->labels[state->label_count].addr =
		(state->current_section == SEC_TEXT) ? state->pc_text : state->pc_data;
	state->label_count++;
}

static void process_directive(assembler_state_t *state, char *s) {
	if (!strncmp(s, ".ascii", 6)) {
		char *q = strchr(s, '"');
		char *r = strrchr(s, '"');
		if (q && r && r > q)
			state->pc_data += (r - q - 1);
	} else if (!strncmp(s, ".byte", 5)) {
		state->pc_data += 1;
	}
}

void first_pass(FILE *f, assembler_state_t *state) {
	char line[MAX_LINE];

	/* Initialize state */
	state->label_count = 0;
	state->pc_text = 0;
	state->pc_data = 0;
	state->current_section = SEC_TEXT;

	while (fgets(line, sizeof(line), f)) {
		char *s = trim(line);
		if (*s == 0 || *s == '#') continue;

		if (!strcmp(s, ".text")) {
			state->current_section = SEC_TEXT;
			continue;
		}

		if (!strcmp(s, ".data")) {
			state->current_section = SEC_DATA;
			continue;
		}

		if (strchr(s, ':')) {
			process_label(state, s);
			continue;
		}

		if (*s == '.') {
			process_directive(state, s);
			continue;
		}

		if (state->current_section == SEC_TEXT) {
			state->pc_text += 4;
		}
	}

	/* relocate data labels */
	for (int i = 0; i < state->label_count; i++) {
		if (state->labels[i].addr < state->pc_text)
			continue;
		state->labels[i].addr += state->pc_text;
	}
}

static uint32_t encode_instruction(const char *op, const char *a1,
								   const char *a2, const char *a3) {
	if (!strcmp(op, "add")) {
		return encode_r(0x00, reg_num(a3), reg_num(a2), 0x0, reg_num(a1), 0x33);
	} else if (!strcmp(op, "addi")) {
		return encode_i(atoi(a3), reg_num(a2), 0x0, reg_num(a1), 0x13);
	} else if (!strcmp(op, "lui")) {
		return encode_u(strtol(a2, NULL, 0), reg_num(a1), 0x37);
	} else if (!strcmp(op, "ecall")) {
		return 0x00000073;
	} else {
		fprintf(stderr, "Unknown instruction: %s\n", op);
		exit(1);
	}
}

static void process_data_directive(FILE *out, char *s, uint32_t *pc) {
	if (!strncmp(s, ".ascii", 6)) {
		char *q = strchr(s, '"');
		char *r = strrchr(s, '"');
		if (!q || !r || r <= q) {
			fprintf(stderr, "Malformed .ascii\n");
			exit(1);
		}
		for (char *p = q + 1; p < r; p++) {
			fwrite(p, 1, 1, out);
			(*pc)++;
		}
	} else if (!strncmp(s, ".byte", 5)) {
		int val;
		if (sscanf(s + 5, "%d", &val) != 1) {
			fprintf(stderr, "Malformed .byte\n");
			exit(1);
		}
		uint8_t b = val & 0xFF;
		fwrite(&b, 1, 1, out);
		(*pc)++;
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
		if (*s == 0 || *s == '#' || strchr(s, ':')) continue;

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

		if (*s == '.') {
			if (current_section == SEC_DATA) {
				process_data_directive(out, s, &pc);
			}
			continue;
		}

		if (current_section == SEC_TEXT) {
			char op[16], a1[32], a2[32], a3[32];
			sscanf(s, "%s %[^,], %[^,], %s", op, a1, a2, a3);
			uint32_t instr = encode_instruction(op, a1, a2, a3);
			fwrite(&instr, 4, 1, out);
			pc += 4;
		}
	}
}
