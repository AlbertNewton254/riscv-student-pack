#include "assembler.h"
#include <stdlib.h>

int main(int argc, char **argv) {
	if (argc != 3) {
		fprintf(stderr, "Usage: %s input.s output.bin\n", argv[0]);
		return 1;
	}

	FILE *in = fopen(argv[1], "r");
	FILE *out = fopen(argv[2], "wb");

	if (!in || !out) {
		perror("file");
		return 1;
	}

	assembler_state_t state;
	first_pass(in, &state);
	second_pass(in, out, &state);

	fclose(in);
	fclose(out);

	printf("Assembled successfully.\n");
	printf("Text: %u bytes, Data: %u bytes, Labels: %d\n",
		   state.pc_text, state.pc_data, state.label_count);
	return 0;
}
