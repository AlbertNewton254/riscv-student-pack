/* adjust_labels.cpp */
#include "assembler.hpp"

void Assembler::adjust_labels(uint32_t data_base) {
	for (size_t i = 0; i < labels.size(); i++) {
		if (labels[i].section == SEC_DATA) {
			labels[i].addr = data_base + labels[i].addr;
		}
	}
}