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
	int n = atoi(r + 1);
	return (n >= 0 && n < 32) ? n : -1;
}

uint32_t find_label(const assembler_state_t *state, const char *name) {
	for (int i = 0; i < state->label_count; i++) {
		if (!strcmp(state->labels[i].name, name))
			return state->labels[i].addr;
	}
	fprintf(stderr, "Undefined label: %s\n", name);
	exit(1);
}
