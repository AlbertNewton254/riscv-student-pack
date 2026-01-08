/* utils.c */
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

	/* Parse only the leading alphanumeric characters of the input. This
	 * allows inputs like "t0," or "a1)" to be accepted by stopping at the
	 * first non-alphanumeric character. */
	size_t len = 0;
	while (r[len] && isalnum((unsigned char)r[len])) len++;
	if (len == 0) return -1;

	char name[16];
	if (len >= sizeof(name)) len = sizeof(name) - 1;
	memcpy(name, r, len);
	name[len] = '\0';

	/* Try ABI names first */
	if (strcmp(name, "zero") == 0) return 0;
	if (strcmp(name, "ra") == 0) return 1;
	if (strcmp(name, "sp") == 0) return 2;
	if (strcmp(name, "gp") == 0) return 3;
	if (strcmp(name, "tp") == 0) return 4;
	if (strcmp(name, "t0") == 0) return 5;
	if (strcmp(name, "t1") == 0) return 6;
	if (strcmp(name, "t2") == 0) return 7;
	if (strcmp(name, "s0") == 0 || strcmp(name, "fp") == 0) return 8;
	if (strcmp(name, "s1") == 0) return 9;
	if (strcmp(name, "a0") == 0) return 10;
	if (strcmp(name, "a1") == 0) return 11;
	if (strcmp(name, "a2") == 0) return 12;
	if (strcmp(name, "a3") == 0) return 13;
	if (strcmp(name, "a4") == 0) return 14;
	if (strcmp(name, "a5") == 0) return 15;
	if (strcmp(name, "a6") == 0) return 16;
	if (strcmp(name, "a7") == 0) return 17;
	if (strcmp(name, "s2") == 0) return 18;
	if (strcmp(name, "s3") == 0) return 19;
	if (strcmp(name, "s4") == 0) return 20;
	if (strcmp(name, "s5") == 0) return 21;
	if (strcmp(name, "s6") == 0) return 22;
	if (strcmp(name, "s7") == 0) return 23;
	if (strcmp(name, "s8") == 0) return 24;
	if (strcmp(name, "s9") == 0) return 25;
	if (strcmp(name, "s10") == 0) return 26;
	if (strcmp(name, "s11") == 0) return 27;
	if (strcmp(name, "t3") == 0) return 28;
	if (strcmp(name, "t4") == 0) return 29;
	if (strcmp(name, "t5") == 0) return 30;
	if (strcmp(name, "t6") == 0) return 31;

	/* Fall back to xN format */
	if (name[0] != 'x') return -1;
	char *end;
	long n = strtol(name + 1, &end, 10);
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

	/* Check for hexadecimal (with optional negative sign) */
	if ((s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) ||
		(s[0] == '-' && s[1] == '0' && (s[2] == 'x' || s[2] == 'X'))) {
		return (int32_t)strtol(s, NULL, 16);
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
