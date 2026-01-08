/* constructor.cpp */
#include "assembler.hpp"

Assembler::Assembler() {
	pc_text = 0;
	pc_data = 0;
	current_section = SEC_TEXT;
	text_size = 0;
	data_size = 0;
}