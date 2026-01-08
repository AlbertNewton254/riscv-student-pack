/* utils.cpp */
#include "assembler.hpp"
#include <cctype>
#include <cstdlib>
#include <cstring>

char *Assembler::trim(char *s) {
	while (isspace(*s)) s++;
	char *end = s + strlen(s) - 1;
	while (end > s && isspace(*end)) *end-- = 0;
	return s;
}

int Assembler::reg_num(const char *r) {
	if (r == NULL || r[0] == '\0') return -1;

	size_t len = 0;
	while (r[len] && isalnum((unsigned char)r[len])) len++;
	if (len == 0) return -1;

	char name[16];
	if (len >= sizeof(name)) len = sizeof(name) - 1;
	memcpy(name, r, len);
	name[len] = '\0';

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

	if (name[0] != 'x') return -1;
	char *end;
	long n = strtol(name + 1, &end, 10);
	if (*end != '\0' || n < 0 || n > 31) return -1;
	return (int)n;
}

uint32_t Assembler::find_label(const char *name) const {
	for (size_t i = 0; i < labels.size(); i++) {
		if (labels[i].name == name)
			return labels[i].addr;
	}
	fprintf(stderr, "Undefined label: %s\n", name);
	exit(1);
}

int32_t Assembler::parse_imm(const char *s) const {
	if (s == NULL || s[0] == '\0') {
		return 0;
	}

	if ((s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) ||
		(s[0] == '-' && s[1] == '0' && (s[2] == 'x' || s[2] == 'X'))) {
		return (int32_t)strtol(s, NULL, 16);
	}

	if (isdigit(s[0]) || (s[0] == '-' && isdigit(s[1]))) {
		return (int32_t)atoi(s);
	}

	return (int32_t)find_label(s);
}

size_t Assembler::parse_escaped_string(const char *src, uint8_t *out) {
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

void Assembler::switch_section(const std::string& section_name) {
	current_section_name = section_name;

	/* Create section if it doesn't exist */
	if (sections.find(section_name) == sections.end()) {
		SectionType type = get_section_type(section_name);
		sections[section_name] = SectionInfo(section_name, type);
	}
}

SectionInfo& Assembler::get_current_section() {
	if (sections.find(current_section_name) == sections.end()) {
		sections[current_section_name] = SectionInfo(current_section_name, SEC_CUSTOM);
	}
	return sections[current_section_name];
}

const SectionInfo& Assembler::get_current_section() const {
	static const SectionInfo default_section;
	auto it = sections.find(current_section_name);
	if (it == sections.end()) {
		return default_section;
	}
	return it->second;
}

SectionType Assembler::get_section_type(const std::string& name) const {
	if (name == ".text" || name.find(".text.") == 0) return SEC_TEXT;
	if (name == ".data" || name.find(".data.") == 0) return SEC_DATA;
	if (name == ".rodata" || name.find(".rodata.") == 0) return SEC_RODATA;
	if (name == ".bss" || name.find(".bss.") == 0) return SEC_BSS;
	return SEC_CUSTOM;
}