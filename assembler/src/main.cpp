/* main.cpp */
#include "assembler.hpp"
#include <cstdlib>
#include <cstring>

int main(int argc, char **argv) {
	bool debug_mode = false;
	const char *input_file = nullptr;
	const char *output_file = nullptr;

	/* Parse command line arguments */
	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "--debug") == 0) {
			debug_mode = true;
		} else if (!input_file) {
			input_file = argv[i];
		} else if (!output_file) {
			output_file = argv[i];
		} else {
			fprintf(stderr, "Error: Too many arguments\n");
			fprintf(stderr, "Usage: %s [--debug] input.s output.bin\n", argv[0]);
			return 1;
		}
	}

	if (!input_file || !output_file) {
		fprintf(stderr, "Usage: %s [--debug] input.s output.bin\n", argv[0]);
		return 1;
	}

	FilePtr in(fopen(input_file, "r"));
	FilePtr out(fopen(output_file, "wb"));

	if (!in || !out) {
		perror("file");
		return 1;
	}

	Assembler assembler;
	assembler.set_debug_mode(debug_mode);

	/* First pass: collect labels and calculate section sizes */
	assembler.first_pass(in.get());

	/* Adjust label addresses: data section comes after text section */
	assembler.adjust_labels(assembler.get_text_size());

	/* Second pass: generate output using adjusted addresses */
	assembler.second_pass(in.get(), out.get());

	printf("Assembled successfully.\n");
	printf("Text: %u bytes, Data: %u bytes, Labels: %zu\n",
		   assembler.get_text_size(), assembler.get_data_size(), assembler.get_label_count());
	return 0;
}