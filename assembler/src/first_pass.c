#include "assembler.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

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

	strcpy(state->labels[state->label_count].name, trimmed);

	/* FIX: Calculate correct address for data labels */
	if (state->current_section == SEC_TEXT) {
		state->labels[state->label_count].addr = state->pc_text;
	} else {
		/* Data labels get address = text_size + current_data_offset */
		state->labels[state->label_count].addr = state->pc_text + state->pc_data;
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

	} else if (!strncmp(s, ".word", 5)) {
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
					state->pc_text += 4;
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
			state->pc_text += 4;
		}
	}
}
