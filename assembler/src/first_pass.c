#include "assembler.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

static int pseudoinstruction_size(const char *op, const char *a2) {
	if (!strcmp(op, "nop") || !strcmp(op, "mv")) {
		return 1;
	}

	if (!strcmp(op, "li")) {
		if (a2 && a2[0] != '\0') {
			if (isdigit(a2[0]) || a2[0] == '-' ||
				(a2[0] == '0' && (a2[1] == 'x' || a2[1] == 'X'))) {
				int32_t imm = parse_imm(NULL, a2);
				if (imm >= -2048 && imm <= 2047) {
					return 1;
				}
			}
		}
		return 2;
	}

	if (!strcmp(op, "la")) {
		return 2;
	}

	return 0;
}

static void parse_simple_args(const char *s, char *op, char *a1, char *a2) {
	/* Initialize outputs */
	op[0] = '\0';
	a1[0] = '\0';
	a2[0] = '\0';

	/* Make a working copy */
	char temp[MAX_LINE];
	strncpy(temp, s, MAX_LINE - 1);
	temp[MAX_LINE - 1] = '\0';

	/* Skip leading whitespace */
	char *ptr = temp;
	while (*ptr && isspace(*ptr)) ptr++;

	if (*ptr == '\0') return;

	/* Extract opcode */
	char *op_end = ptr;
	while (*op_end && !isspace(*op_end)) op_end++;

	size_t op_len = op_end - ptr;
	if (op_len >= 16) op_len = 15;  /* Leave room for null terminator */
	strncpy(op, ptr, op_len);
	op[op_len] = '\0';

	ptr = op_end;
	while (*ptr && isspace(*ptr)) ptr++;

	if (*ptr == '\0') return;

	/* Find comma for argument separation */
	char *comma = strchr(ptr, ',');

	if (!comma) {
		/* Single argument */
		size_t len = strlen(ptr);
		if (len >= 31) len = 30;  /* Leave room for null terminator */
		strncpy(a1, ptr, len);
		a1[len] = '\0';
		trim(a1);
	} else {
		/* Two arguments separated by comma */
		*comma = '\0';

		size_t len1 = strlen(ptr);
		if (len1 >= 31) len1 = 30;
		strncpy(a1, ptr, len1);
		a1[len1] = '\0';
		trim(a1);

		char *arg2 = comma + 1;
		while (*arg2 && isspace(*arg2)) arg2++;

		if (*arg2) {
			size_t len2 = strlen(arg2);
			if (len2 >= 31) len2 = 30;
			strncpy(a2, arg2, len2);
			a2[len2] = '\0';
			trim(a2);
		}
	}
}

static void process_instruction_first_pass(assembler_state_t *state, const char *s) {
	char op[16] = "", a1[32] = "", a2[32] = "";
	parse_simple_args(s, op, a1, a2);

	int pseudo_size = pseudoinstruction_size(op, a2);
	if (pseudo_size > 0) {
		state->pc_text += pseudo_size * 4;
	} else {
		state->pc_text += 4;
	}
}

static void process_label(assembler_state_t *state, char *s) {
	char *colon = strchr(s, ':');
	if (!colon) return;

	char *label_start = s;
	while (label_start < colon && isspace(*label_start)) {
		label_start++;
	}

	if (label_start == colon) {
		return;
	}

	char label_name[64];
	int i = 0;
	while (label_start < colon && i < 63) {
		label_name[i++] = *label_start++;
	}
	label_name[i] = '\0';

	char *trimmed = trim(label_name);

	for (int i = 0; i < state->label_count; i++) {
		if (!strcmp(state->labels[i].name, trimmed)) {
			fprintf(stderr, "Duplicate label: %s\n", trimmed);
			exit(1);
		}
	}

	if (state->label_count >= MAX_LABELS) {
		fprintf(stderr, "Too many labels\n");
		exit(1);
	}

	strcpy(state->labels[state->label_count].name, trimmed);
	state->labels[state->label_count].section = state->current_section;

	if (state->current_section == SEC_TEXT) {
		state->labels[state->label_count].addr = state->pc_text;
	} else {
		state->labels[state->label_count].addr = state->pc_data;
	}

	state->label_count++;
}

static void process_directive(assembler_state_t *state, char *s) {
	s = trim(s);

	if (!strncmp(s, ".ascii", 6)) {
		char *q = strchr(s, '"');
		if (!q) {
			fprintf(stderr, "Malformed .ascii\n");
			exit(1);
		}
		state->pc_data += parse_escaped_string(q + 1, NULL);

	} else if (!strncmp(s, ".asciiz", 7)) {
		char *q = strchr(s, '"');
		if (!q) {
			fprintf(stderr, "Malformed .asciiz\n");
			exit(1);
		}
		state->pc_data += parse_escaped_string(q + 1, NULL) + 1;

	} else if (!strncmp(s, ".byte", 5)) {
		char *ptr = s + 5;
		int count = 0;
		while (*ptr) {
			if (*ptr != ' ' && *ptr != '\t' && *ptr != ',') {
				count++;
				while (*ptr && *ptr != ',') ptr++;
			}
			if (*ptr) ptr++;
		}
		state->pc_data += count;

	} else if (!strncmp(s, ".half", 5)) {
		char *ptr = s + 5;
		int count = 0;
		while (*ptr) {
			if (*ptr != ' ' && *ptr != '\t' && *ptr != ',') {
				count++;
				while (*ptr && *ptr != ',') ptr++;
			}
			if (*ptr) ptr++;
		}
		state->pc_data += count * 2;

	} else if (!strcmp(s, ".word") || !strncmp(s, ".word", 5)) {
		char *ptr = s + 5;
		int count = 0;
		while (*ptr) {
			if (*ptr != ' ' && *ptr != '\t' && *ptr != ',') {
				count++;
				while (*ptr && *ptr != ',') ptr++;
			}
			if (*ptr) ptr++;
		}
		state->pc_data += count * 4;

	} else if (!strncmp(s, ".space", 6)) {
		int size;
		if (sscanf(s + 6, "%d", &size) == 1)
			state->pc_data += size;
	}
}

void first_pass(FILE *f, assembler_state_t *state) {
	char line[MAX_LINE];

	state->label_count = 0;
	state->pc_text = 0;
	state->pc_data = 0;
	state->current_section = SEC_TEXT;

	while (fgets(line, sizeof(line), f)) {
		/* Ensure null termination */
		line[MAX_LINE - 1] = '\0';

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

		if (strstr(s, ".globl")) {
			continue;
		}

		char *colon = strchr(s, ':');
		if (colon) {
			process_label(state, s);
			char *after_colon = colon + 1;
			while (*after_colon && isspace(*after_colon)) {
				after_colon++;
			}
			if (*after_colon && *after_colon != '#') {
				if (state->current_section == SEC_TEXT) {
					process_instruction_first_pass(state, after_colon);
				} else if (*after_colon == '.') {
					process_directive(state, after_colon);
				}
			}
			continue;
		}

		if (*s == '.') {
			process_directive(state, s);
			continue;
		}

		if (state->current_section == SEC_TEXT) {
			process_instruction_first_pass(state, s);
		}
	}

	state->text_size = state->pc_text;
	state->data_size = state->pc_data;
}
