/* first_pass.cpp */
#include "assembler.hpp"
#include <cctype>
#include <cstdlib>
#include <cstring>

static void safe_strncpy(char *dest, const char *src, size_t n) {
	if (n == 0) return;
	size_t i;
	for (i = 0; i < n - 1 && src[i] != '\0'; i++) {
		dest[i] = src[i];
	}
	dest[i] = '\0';
}

int Assembler::pseudoinstruction_size(const char *op, const char *a2) const {
	if (!strcmp(op, "nop") || !strcmp(op, "mv")) {
		return 1;
	}

	if (!strcmp(op, "li")) {
		if (a2 && a2[0] != '\0') {
			if (isdigit(a2[0]) || a2[0] == '-' ||
				(a2[0] == '0' && (a2[1] == 'x' || a2[1] == 'X'))) {
				int32_t imm = parse_imm(a2);
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

void Assembler::parse_simple_args(const char *s, char *op, char *a1, char *a2) const {
	op[0] = '\0';
	a1[0] = '\0';
	a2[0] = '\0';

	char temp[MAX_LINE];
	safe_strncpy(temp, s, MAX_LINE);
	temp[MAX_LINE - 1] = '\0';

	char *ptr = temp;
	while (*ptr && isspace(*ptr)) ptr++;

	if (*ptr == '\0') return;

	char *op_end = ptr;
	while (*op_end && !isspace(*op_end)) op_end++;

	size_t op_len = op_end - ptr;
	if (op_len >= 16) op_len = 15;
	safe_strncpy(op, ptr, op_len + 1);

	ptr = op_end;
	while (*ptr && isspace(*ptr)) ptr++;

	if (*ptr == '\0') return;

	char *comma = strchr(ptr, ',');

	if (!comma) {
		size_t len = strlen(ptr);
		if (len >= 31) len = 30;
		safe_strncpy(a1, ptr, len + 1);
		trim(a1);
	} else {
		*comma = '\0';

		size_t len1 = strlen(ptr);
		if (len1 >= 31) len1 = 30;
		safe_strncpy(a1, ptr, len1 + 1);
		trim(a1);

		char *arg2 = comma + 1;
		while (*arg2 && isspace(*arg2)) arg2++;

		if (*arg2) {
			size_t len2 = strlen(arg2);
			if (len2 >= 31) len2 = 30;
			safe_strncpy(a2, arg2, len2 + 1);
			trim(a2);
		}
	}
}

void Assembler::process_instruction_first_pass(const char *s) {
	char op[16] = "", a1[32] = "", a2[32] = "";
	parse_simple_args(s, op, a1, a2);

	int pseudo_size = pseudoinstruction_size(op, a2);
	if (pseudo_size > 0) {
		get_current_section().offset += pseudo_size * 4;
		/* Keep pc_text for backwards compatibility */
		if (current_section_name == ".text") {
			pc_text += pseudo_size * 4;
		}
	} else {
		get_current_section().offset += 4;
		if (current_section_name == ".text") {
			pc_text += 4;
		}
	}
}

void Assembler::process_label(char *s) {
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

	for (size_t i = 0; i < labels.size(); i++) {
		if (labels[i].name == trimmed) {
			fprintf(stderr, "Duplicate label: %s\n", trimmed);
			exit(1);
		}
	}

	Label new_label;
	new_label.name = trimmed;
	new_label.section_name = current_section_name;
	new_label.addr = get_current_section().offset;

	labels.push_back(new_label);
}

void Assembler::process_directive(char *s) {
	s = trim(s);

	if (!strncmp(s, ".ascii", 6)) {
		char *q = strchr(s, '"');
		if (!q) {
			fprintf(stderr, "Malformed .ascii\n");
			exit(1);
		}
		uint32_t size = parse_escaped_string(q + 1, NULL);
		get_current_section().offset += size;
		/* Keep pc_data for backwards compatibility */
		if (current_section_name == ".data") {
			pc_data += size;
		}

	} else if (!strncmp(s, ".asciiz", 7)) {
		char *q = strchr(s, '"');
		if (!q) {
			fprintf(stderr, "Malformed .asciiz\n");
			exit(1);
		}
		uint32_t size = parse_escaped_string(q + 1, NULL) + 1;
		get_current_section().offset += size;
		if (current_section_name == ".data") {
			pc_data += size;
		}

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
		get_current_section().offset += count;
		if (current_section_name == ".data") {
			pc_data += count;
		}

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
		get_current_section().offset += count * 2;
		if (current_section_name == ".data") {
			pc_data += count * 2;
		}

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
		get_current_section().offset += count * 4;
		if (current_section_name == ".data") {
			pc_data += count * 4;
		}

	} else if (!strncmp(s, ".space", 6)) {
		int size;
		if (sscanf(s + 6, "%d", &size) == 1) {
			get_current_section().offset += size;
			if (current_section_name == ".data") {
				pc_data += size;
			}
		}
	}
}

void Assembler::first_pass(FILE *f) {
	char line[MAX_LINE];

	labels.clear();
	pc_text = 0;
	pc_data = 0;

	while (fgets(line, sizeof(line), f)) {
		line[MAX_LINE - 1] = '\0';

		char *s = trim(line);
		if (*s == 0 || *s == '#') continue;

		/* Handle .section directive with optional arguments */
		if (!strncmp(s, ".section", 8)) {
			char *section_start = s + 8;
			while (*section_start && isspace(*section_start)) section_start++;

			/* Extract section name (until comma, space, or end of line) */
			char section_name[64] = "";
			int i = 0;
			while (section_start[i] && !isspace(section_start[i]) &&
			       section_start[i] != ',' && i < 63) {
				section_name[i] = section_start[i];
				i++;
			}
			section_name[i] = '\0';

			if (section_name[0]) {
				switch_section(section_name);
			}
			continue;
		}

		/* Handle legacy .text and .data directives */
		if (!strcmp(s, ".text")) {
			switch_section(".text");
			continue;
		}

		if (!strcmp(s, ".data")) {
			switch_section(".data");
			continue;
		}

		/* Handle .rodata and .bss directives */
		if (!strcmp(s, ".rodata")) {
			switch_section(".rodata");
			continue;
		}

		if (!strcmp(s, ".bss")) {
			switch_section(".bss");
			continue;
		}

		if (strstr(s, ".globl")) {
			continue;
		}

		char *colon = strchr(s, ':');
		if (colon) {
			process_label(s);
			char *after_colon = colon + 1;
			while (*after_colon && isspace(*after_colon)) {
				after_colon++;
			}
			if (*after_colon && *after_colon != '#') {
				SectionType sec_type = get_current_section().type;
				if (sec_type == SEC_TEXT) {
					process_instruction_first_pass(after_colon);
				} else if (*after_colon == '.') {
					process_directive(after_colon);
				}
			}
			continue;
		}

		if (*s == '.') {
			process_directive(s);
			continue;
		}

		SectionType sec_type = get_current_section().type;
		if (sec_type == SEC_TEXT) {
			process_instruction_first_pass(s);
		}
	}

	text_size = pc_text;
	data_size = pc_data;
}