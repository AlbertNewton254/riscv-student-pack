/* adjust_labels.c */
#include "assembler.hpp"

void adjust_labels(assembler_state_t *state, uint32_t data_base) {
	for (size_t i = 0; i < state->labels.size(); i++) {
		if (state->labels[i].section == SEC_DATA) {
			/* Data labels: base address + offset within data section */
			state->labels[i].addr = data_base + state->labels[i].addr;
		}
		/* Text labels: already have correct absolute addresses starting from 0 */
	}
}