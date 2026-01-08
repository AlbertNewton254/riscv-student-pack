/* constructor.cpp */
#include "assembler.hpp"

Assembler::Assembler() {
	pc_text = 0;
	pc_data = 0;
	text_size = 0;
	data_size = 0;
	current_section_name = ".text";

	/* Initialize standard sections */
	sections[".text"] = SectionInfo(".text", SEC_TEXT);
	sections[".data"] = SectionInfo(".data", SEC_DATA);
	sections[".rodata"] = SectionInfo(".rodata", SEC_RODATA);
	sections[".bss"] = SectionInfo(".bss", SEC_BSS);
}