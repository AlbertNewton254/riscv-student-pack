/* adjust_labels.cpp */
#include "assembler.hpp"

void Assembler::adjust_labels(uint32_t data_base) {
	(void)data_base;  /* Parameter kept for backwards compatibility */

	/* Calculate base addresses for all sections */
	uint32_t current_base = 0;

	/* .text section starts at 0 */
	if (sections.find(".text") != sections.end()) {
		sections[".text"].base_addr = 0;
		current_base = sections[".text"].offset;
	}

	/* .rodata section comes after .text */
	if (sections.find(".rodata") != sections.end()) {
		sections[".rodata"].base_addr = current_base;
		current_base += sections[".rodata"].offset;
	}

	/* .data section comes after .rodata (or .text if no .rodata) */
	if (sections.find(".data") != sections.end()) {
		sections[".data"].base_addr = current_base;
		current_base += sections[".data"].offset;
	}

	/* .bss section comes last */
	if (sections.find(".bss") != sections.end()) {
		sections[".bss"].base_addr = current_base;
		current_base += sections[".bss"].offset;
	}

	/* Assign base addresses to custom sections */
	for (auto& entry : sections) {
		SectionInfo& sec = entry.second;
		if (sec.type == SEC_CUSTOM && sec.base_addr == 0) {
			sec.base_addr = current_base;
			current_base += sec.offset;
		}
	}

	/* Adjust label addresses based on their section */
	for (size_t i = 0; i < labels.size(); i++) {
		auto it = sections.find(labels[i].section_name);
		if (it != sections.end()) {
			labels[i].addr += it->second.base_addr;
		}
	}
}