/* test_assembler.cpp */
#include "../include/assembler.hpp"
#include <cstdio>
#include <cstring>
#include <cassert>

/* Test helper functions */
static void test_trim(void) {
	printf("Test 1: trim() function...\n");
	char test1[] = "  hello  ";
	char test2[] = "\t\n  world\t\n";
	char test3[] = "no_spaces";
	char test4[] = "";

	char *result = Assembler::trim(test1);
	assert(strcmp(result, "hello") == 0);

	result = Assembler::trim(test2);
	assert(strcmp(result, "world") == 0);

	result = Assembler::trim(test3);
	assert(strcmp(result, "no_spaces") == 0);

	result = Assembler::trim(test4);
	assert(strcmp(result, "") == 0);

	printf("\tOK trim() works\n");
}

static void test_reg_num(void) {
	printf("Test 2: reg_num() function...\n");

	/* Test xN format */
	assert(Assembler::reg_num("x0") == 0);
	assert(Assembler::reg_num("x31") == 31);
	assert(Assembler::reg_num("x10") == 10);

	/* Test ABI names */
	assert(Assembler::reg_num("zero") == 0);
	assert(Assembler::reg_num("ra") == 1);
	assert(Assembler::reg_num("sp") == 2);
	assert(Assembler::reg_num("gp") == 3);
	assert(Assembler::reg_num("tp") == 4);
	assert(Assembler::reg_num("t0") == 5);
	assert(Assembler::reg_num("t1") == 6);
	assert(Assembler::reg_num("s0") == 8);
	assert(Assembler::reg_num("fp") == 8);
	assert(Assembler::reg_num("a0") == 10);
	assert(Assembler::reg_num("a7") == 17);

	/* Test with trailing characters (should parse only alphanumeric) */
	assert(Assembler::reg_num("x1,") == 1);
	assert(Assembler::reg_num("a0)") == 10);

	/* Test invalid registers */
	assert(Assembler::reg_num("x32") == -1);
	assert(Assembler::reg_num("invalid") == -1);
	assert(Assembler::reg_num("") == -1);

	printf("\tOK reg_num() works\n");
}

static void test_encode_functions(void) {
	printf("Test 3: Instruction encoding functions...\n");

	/* Test encode_r */
	uint32_t instr = Encoder::encode_r(0x00, 3, 2, 0x0, 1, 0x33);
	assert(instr == 0x003100B3); /* add x1, x2, x3 */

	/* Test encode_i */
	instr = Encoder::encode_i(42, 2, 0x0, 1, 0x13);
	assert(instr == 0x02A10093); /* addi x1, x2, 42 */

	/* Test encode_s */
	instr = Encoder::encode_s(4, 3, 2, 0x2, 0x23);
	assert(instr == 0x00312223); /* sw x3, 4(x2) */

	/* Test encode_b */
	instr = Encoder::encode_b(8, 3, 2, 0x0, 0x63);
	assert(instr == 0x00310463); /* beq x2, x3, 8 */

	/* Test encode_u */
	instr = Encoder::encode_u(0x12345000, 1, 0x37);
	assert(instr == 0x123450B7); /* lui x1, 0x12345 */

	/* Test encode_j */
	instr = Encoder::encode_j(1024, 1, 0x6F);
	assert(instr == 0x400000EF); /* jal x1, 1024 */

	printf("\tOK all encode functions work\n");
}

static void test_parse_escaped_string(void) {
	printf("Test 4: parse_escaped_string() function...\n");

	const char *input = "Hello\\nWorld\\t\\\"Test\\\"";
	uint8_t output[64];

	size_t len = Assembler::parse_escaped_string(input, NULL);
	assert(len == 18); /* Count mode */

	len = Assembler::parse_escaped_string(input, output);
	assert(len == 18);

	/* Verify specific characters */
	assert(output[5] == '\n');
	assert(output[11] == '\t');
	assert(output[12] == '"');
	assert(output[17] == '"');

	/* Test simple string */
	const char *simple = "Hello";
	len = Assembler::parse_escaped_string(simple, NULL);
	assert(len == 5);

	printf("\tOK parse_escaped_string() works\n");
}

static void test_first_pass_basic(void) {
	printf("Test 5: Basic first pass functionality...\n");

	/* Create a test assembly file in memory */
	const char *assembly =
		".text\n"
		"start:\n"
		"	addi x1, x0, 42\n"
		"	li x2, 0x100\n"
		"loop:\n"
		"	addi x1, x1, -1\n"
		"	bne x1, x0, loop\n"
		".data\n"
		"msg:\n"
		"	.ascii \"Hello\\n\"\n"
		"count:\n"
		"	.word 0x12345678\n";

	FILE *temp = tmpfile();
	fputs(assembly, temp);
	rewind(temp);

	Assembler assembler;
	assembler.first_pass(temp);

	assert(assembler.get_label_count() >= 3); /* start, loop, msg, count */
	assert(assembler.get_text_size() > 0);
	assert(assembler.get_data_size() > 0);

	fclose(temp);
	printf("\tOK first pass works\n");
}

static void test_adjust_labels(void) {
	printf("Test 6: adjust_labels() function...\n");

	const char *assembly =
		".text\n"
		"text_label:\n"
		"	nop\n"
		"another_text:\n"
		"	nop\n"
		".data\n"
		"data_label:\n"
		"	.word 1\n";

	FILE *temp = tmpfile();
	fputs(assembly, temp);
	rewind(temp);

	Assembler assembler;
	assembler.first_pass(temp);

	uint32_t text_size = assembler.get_text_size();
	assert(text_size > 0);

	/* Adjust labels */
	assembler.adjust_labels(text_size);

	/* Labels should now be adjusted (data labels moved past text section) */
	assert(assembler.get_label_count() >= 3);

	fclose(temp);
	printf("\tOK adjust_labels() works\n");
}

static void parse_instruction_for_test(const char *input, char *op, char *a1, char *a2, char *a3) {
	memset(op, 0, 16);
	memset(a1, 0, 32);
	memset(a2, 0, 32);
	memset(a3, 0, 32);

	char buffer[128];
	strncpy(buffer, input, sizeof(buffer) - 1);
	buffer[sizeof(buffer) - 1] = '\0';

	char *args = strchr(buffer, ' ');
	if (!args) {
		strcpy(op, buffer);
		return;
	}

	strncpy(op, buffer, args - buffer);
	op[args - buffer] = '\0';

	char *ptr = args + 1;
	char *comma1 = strchr(ptr, ',');
	char *comma2 = comma1 ? strchr(comma1 + 1, ',') : NULL;

	if (!comma1) {
		strcpy(a1, Assembler::trim(ptr));
	} else if (!comma2) {
		*comma1 = '\0';
		strcpy(a1, Assembler::trim(ptr));
		strcpy(a2, Assembler::trim(comma1 + 1));
	} else {
		*comma1 = '\0';
		strcpy(a1, Assembler::trim(ptr));

		char *middle = comma1 + 1;
		*comma2 = '\0';
		strcpy(a2, Assembler::trim(middle));
		strcpy(a3, Assembler::trim(comma2 + 1));
	}
}

static void test_instruction_parsing(void) {
	printf("Test 7: Instruction argument parsing...\n");

	char op[16], a1[32], a2[32], a3[32];

	parse_instruction_for_test("add x1, x2, x3", op, a1, a2, a3);
	assert(strcmp(op, "add") == 0);
	assert(strcmp(a1, "x1") == 0);
	assert(strcmp(a2, "x2") == 0);
	assert(strcmp(a3, "x3") == 0);

	parse_instruction_for_test("addi x1, x2, 42", op, a1, a2, a3);
	assert(strcmp(op, "addi") == 0);
	assert(strcmp(a1, "x1") == 0);
	assert(strcmp(a2, "x2") == 0);
	assert(strcmp(a3, "42") == 0);

	printf("\tOK instruction parsing works\n");
}

static void test_section_switching(void) {
	printf("Test 8: Section switching...\n");

	const char *assembly =
		".text\n"
		"	nop\n"
		".data\n"
		"	.word 1\n"
		".text\n"
		"	addi x1, x0, 1\n"
		".data\n"
		"	.word 2\n";

	FILE *temp = tmpfile();
	fputs(assembly, temp);
	rewind(temp);

	Assembler assembler;
	assembler.first_pass(temp);

	assert(assembler.get_text_size() == 8); /* nop + addi = 2 instructions */
	assert(assembler.get_data_size() == 8); /* 2 words = 8 bytes */

	fclose(temp);
	printf("\tOK section switching works\n");
}

static void test_full_assembly(void) {
	printf("Test 9: Full assembly process...\n");

	const char *assembly =
		".text\n"
		"main:\n"
		"	addi x1, x0, 5\n"
		"	addi x2, x0, 10\n"
		"	add x3, x1, x2\n"
		"	jal x1, end\n"
		"end:\n"
		"	addi x0, x0, 0\n"
		".data\n"
		"value:\n"
		"	.word 42\n";

	FILE *in = tmpfile();
	FILE *out = tmpfile();
	fputs(assembly, in);
	rewind(in);

	Assembler assembler;
	assembler.first_pass(in);
	assembler.adjust_labels(assembler.get_text_size());
	assembler.second_pass(in, out);

	/* Verify output size */
	fseek(out, 0, SEEK_END);
	long size = ftell(out);
	assert(size > 0);

	fclose(in);
	fclose(out);
	printf("\tOK full assembly works\n");
}

static void test_pseudoinstructions(void) {
	printf("Test 10: Pseudoinstruction expansion...\n");

	const char *assembly =
		".text\n"
		"	nop\n"
		"	li x1, 42\n"
		"	li x2, 0x12345\n"
		"	mv x3, x4\n";

	FILE *in = tmpfile();
	FILE *out = tmpfile();
	fputs(assembly, in);
	rewind(in);

	Assembler assembler;
	assembler.first_pass(in);

	/* nop (1 instr) + li 42 (1 instr) + li 0x12345 (2 instr) + mv (1 instr) = 5 instructions = 20 bytes */
	assert(assembler.get_text_size() == 20);

	assembler.adjust_labels(assembler.get_text_size());
	assembler.second_pass(in, out);

	fclose(in);
	fclose(out);
	printf("\tOK pseudoinstruction expansion works\n");
}

int main(void) {
	printf("=== RISC-V Assembler Comprehensive Tests ===\n\n");

	test_trim();
	test_reg_num();
	test_encode_functions();
	test_parse_escaped_string();
	test_first_pass_basic();
	test_adjust_labels();
	test_instruction_parsing();
	test_section_switching();
	test_full_assembly();
	test_pseudoinstructions();

	printf("\n=== All %d tests passed! ===\n", 10);
	return 0;
}
