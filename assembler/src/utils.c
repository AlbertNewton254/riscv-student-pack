#include "assembler.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

char *trim(char *s) {
	while (isspace(*s)) s++;
	char *end = s + strlen(s) - 1;
	while (end > s && isspace(*end)) *end-- = 0;
	return s;
}

int reg_num(const char *r) {
	if (r == NULL || r[0] == '\0') return -1;

	/* Try ABI names first */
	if (strcmp(r, "zero") == 0) return 0;
	if (strcmp(r, "ra") == 0) return 1;
	if (strcmp(r, "sp") == 0) return 2;
	if (strcmp(r, "gp") == 0) return 3;
	if (strcmp(r, "tp") == 0) return 4;
	if (strcmp(r, "t0") == 0) return 5;
	if (strcmp(r, "t1") == 0) return 6;
	if (strcmp(r, "t2") == 0) return 7;
	if (strcmp(r, "s0") == 0 || strcmp(r, "fp") == 0) return 8;
	if (strcmp(r, "s1") == 0) return 9;
	if (strcmp(r, "a0") == 0) return 10;
	if (strcmp(r, "a1") == 0) return 11;
	if (strcmp(r, "a2") == 0) return 12;
	if (strcmp(r, "a3") == 0) return 13;
	if (strcmp(r, "a4") == 0) return 14;
	if (strcmp(r, "a5") == 0) return 15;
	if (strcmp(r, "a6") == 0) return 16;
	if (strcmp(r, "a7") == 0) return 17;
	if (strcmp(r, "s2") == 0) return 18;
	if (strcmp(r, "s3") == 0) return 19;
	if (strcmp(r, "s4") == 0) return 20;
	if (strcmp(r, "s5") == 0) return 21;
	if (strcmp(r, "s6") == 0) return 22;
	if (strcmp(r, "s7") == 0) return 23;
	if (strcmp(r, "s8") == 0) return 24;
	if (strcmp(r, "s9") == 0) return 25;
	if (strcmp(r, "s10") == 0) return 26;
	if (strcmp(r, "s11") == 0) return 27;
	if (strcmp(r, "t3") == 0) return 28;
	if (strcmp(r, "t4") == 0) return 29;
	if (strcmp(r, "t5") == 0) return 30;
	if (strcmp(r, "t6") == 0) return 31;

	/* Fall back to xN format */
	if (r[0] != 'x') return -1;
	char *end;
	long n = strtol(r + 1, &end, 10);
	if (*end != '\0' || n < 0 || n > 31) return -1;
	return (int)n;
}

uint32_t find_label(const assembler_state_t *state, const char *name) {
	for (int i = 0; i < state->label_count; i++) {
		if (!strcmp(state->labels[i].name, name))
			return state->labels[i].addr;
	}
	fprintf(stderr, "Undefined label: %s\n", name);
	exit(1);
}

int32_t parse_imm(const assembler_state_t *state, const char *s) {
	if (s == NULL || s[0] == '\0') {
		return 0;
	}

	/* Check for hexadecimal */
	if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
		return (int32_t)strtoul(s, NULL, 16);
	}

	/* Check for decimal (positive or negative) */
	if (isdigit(s[0]) || (s[0] == '-' && isdigit(s[1]))) {
		return (int32_t)atoi(s);
	}

	/* If state is NULL, we're in first pass */
	if (state == NULL) {
		/* For first pass, labels are not resolved yet */
		return 0;
	}

	/* Otherwise, treat as label */
	return (int32_t)find_label(state, s);
}

size_t parse_escaped_string(const char *src, uint8_t *out) {
	size_t count = 0;

	for (size_t i = 0; src[i] && src[i] != '"'; i++) {
		uint8_t c;

		if (src[i] == '\\') {
			i++;
			switch (src[i]) {
				case 'n':  c = '\n'; break;
				case 't':  c = '\t'; break;
				case 'r':  c = '\r'; break;
				case '\\': c = '\\'; break;
				case '"':  c = '"';  break;
				default:
					fprintf(stderr, "Unknown escape: \\%c\n", src[i]);
					exit(1);
			}
		} else {
			c = (uint8_t)src[i];
		}

		if (out)
			out[count] = c;

		count++;
	}

	return count;
}
