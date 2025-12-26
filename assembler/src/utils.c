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

	if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
		return (int32_t)strtoul(s, NULL, 16);
	}

	if (isdigit(s[0]) || (s[0] == '-' && isdigit(s[1]))) {
		return (int32_t)atoi(s);
	}

	if (state == NULL) {
		return (int32_t)strtoul(s, NULL, 0);
	}

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
