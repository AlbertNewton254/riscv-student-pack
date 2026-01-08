/* main.cpp */
#include "assembler.hpp"
#include <cstdlib>

int main(int argc, char **argv) {
	if (argc != 3) {
		fprintf(stderr, "Usage: %s input.s output.bin\n", argv[0]);
		return 1;
	}

	FilePtr in(fopen(argv[1], "r"));
	FilePtr out(fopen(argv[2], "wb"));

	if (!in || !out) {
		perror("file");
		return 1;
	}

	Assembler assembler;

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