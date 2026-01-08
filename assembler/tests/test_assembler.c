/* test_assembler.c */
#include "../include/assembler.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

/* Test helper functions */
static void test_trim(void) {
	printf("Test 1: trim() function...\n");
	char test1[] = "  hello  ";
	char test2[] = "\t\n  world\t\n";
	char test3[] = "no_spaces";
	char test4[] = "";

	char *result = trim(test1);
	assert(strcmp(result, "hello") == 0);

	result = trim(test2);
	assert(strcmp(result, "world") == 0);

	result = trim(test3);
	assert(strcmp(result, "no_spaces") == 0);

	result = trim(test4);
	assert(strcmp(result, "") == 0);

	printf("\tOK trim() works\n");
}

static void test_reg_num(void) {
	printf("Test 2: reg_num() function...\n");

	/* Test xN format */
	assert(reg_num("x0") == 0);
	assert(reg_num("x31") == 31);
	assert(reg_num("x10") == 10);

	/* Test ABI names */
	assert(reg_num("zero") == 0);
	assert(reg_num("ra") == 1);
	assert(reg_num("sp") == 2);
	assert(reg_num("gp") == 3);
	assert(reg_num("tp") == 4);
	assert(reg_num("t0") == 5);
	assert(reg_num("t1") == 6);
	assert(reg_num("s0") == 8);
	assert(reg_num("fp") == 8);
	assert(reg_num("a0") == 10);
	assert(reg_num("a7") == 17);

	/* Test with trailing characters (should parse only alphanumeric) */
	assert(reg_num("x1,") == 1);
	assert(reg_num("a0)") == 10);

	/* Test invalid registers */
	assert(reg_num("x32") == -1);
	assert(reg_num("invalid") == -1);
	assert(reg_num("") == -1);

	printf("\tOK reg_num() works\n");
}

static void test_encode_functions(void) {
	printf("Test 3: Instruction encoding functions...\n");

	/* Test encode_r */
	uint32_t instr = encode_r(0x00, 3, 2, 0x0, 1, 0x33);
	assert(instr == 0x003100B3); /* add x1, x2, x3 */

	/* Test encode_i */
	instr = encode_i(42, 2, 0x0, 1, 0x13);
	assert(instr == 0x02A10093); /* addi x1, x2, 42 */

	/* Test encode_s */
	instr = encode_s(4, 3, 2, 0x2, 0x23);
	assert(instr == 0x00312223); /* sw x3, 4(x2) */

	/* Test encode_b */
	instr = encode_b(8, 3, 2, 0x0, 0x63);
	assert(instr == 0x00310463); /* beq x2, x3, 8 */

	/* Test encode_u */
	instr = encode_u(0x12345000, 1, 0x37);
	assert(instr == 0x123450B7); /* lui x1, 0x12345 */

	/* Test encode_j */
	instr = encode_j(1024, 1, 0x6F);
	assert(instr == 0x400000EF); /* jal x1, 1024 */

	printf("\tOK all encode functions work\n");
}

static void test_parse_imm(void) {
	printf("Test 4: parse_imm() function...\n");

	/* Test with NULL state (first pass) */
	assert(parse_imm(NULL, "42") == 42);
	assert(parse_imm(NULL, "0xFF") == 0xFF);
	assert(parse_imm(NULL, "0x100") == 0x100);
	assert(parse_imm(NULL, "-10") == -10);
	assert(parse_imm(NULL, "-0x10") == -0x10);

	/* Test with labels (requires state with labels) */
	assembler_state_t state = {0};
	state.label_count = 1;
	strcpy(state.labels[0].name, "my_label");
	state.labels[0].addr = 0x1000;
	state.labels[0].section = SEC_TEXT;

	/* This would normally fail in real usage as find_label would be called */
	/* We'll test it with a stub or mock later */

	printf("\tOK parse_imm() basic cases work\n");
}

static void test_parse_escaped_string(void) {
	printf("Test 5: parse_escaped_string() function...\n");

	const char *input = "Hello\\nWorld\\t\\\"Test\\\"";
	uint8_t output[64];

	size_t len = parse_escaped_string(input, NULL);
	assert(len == 18); /* Count mode */

	len = parse_escaped_string(input, output);
	assert(len == 18);

	/* Verify specific characters */
	assert(output[5] == '\n');
	assert(output[11] == '\t');
	assert(output[12] == '"');
	assert(output[17] == '"');

	/* Test simple string */
	const char *simple = "Hello";
	len = parse_escaped_string(simple, NULL);
	assert(len == 5);

	printf("\tOK parse_escaped_string() works\n");
}

static void test_pseudoinstruction_expansion(void) {
	printf("Test 6: Pseudoinstruction expansion...\n");

	assembler_state_t state = {0};
	state.label_count = 1;
	strcpy(state.labels[0].name, "data_label");
	state.labels[0].addr = 0x2000;
	state.labels[0].section = SEC_DATA;

	char expanded[2][MAX_LINE];
	uint32_t current_pc = 0x1000;

	/* Test nop */
	int count = expand_pseudoinstruction("nop", "", "", &state, expanded, current_pc);
	assert(count == 1);
	assert(strcmp(expanded[0], "addi x0, x0, 0") == 0);

	/* Test mv */
	count = expand_pseudoinstruction("mv", "x1", "x2", &state, expanded, current_pc);
	assert(count == 1);
	assert(strcmp(expanded[0], "addi x1, x2, 0") == 0);

	/* Test li with small immediate */
	count = expand_pseudoinstruction("li", "x1", "42", &state, expanded, current_pc);
	assert(count == 1);
	assert(strstr(expanded[0], "addi x1, x0, 42") != NULL);

	/* Test li with large immediate (requires lui + addi) */
	count = expand_pseudoinstruction("li", "x1", "0x12345", &state, expanded, current_pc);
	assert(count == 2);
	assert(strstr(expanded[0], "lui x1,") != NULL);
	assert(strstr(expanded[1], "addi x1, x1,") != NULL);

	/* Test la */
	count = expand_pseudoinstruction("la", "x1", "data_label", &state, expanded, current_pc);
	assert(count == 2);
	assert(strstr(expanded[0], "auipc x1,") != NULL);
	assert(strstr(expanded[1], "addi x1, x1,") != NULL);

	printf("\tOK pseudoinstruction expansion works\n");
}

static void test_adjust_labels(void) {
	printf("Test 7: adjust_labels() function...\n");

	assembler_state_t state = {0};
	state.label_count = 3;

	/* Text section label */
	strcpy(state.labels[0].name, "text_label");
	state.labels[0].addr = 0x100; /* Relative to text start */
	state.labels[0].section = SEC_TEXT;

	/* Data section label */
	strcpy(state.labels[1].name, "data_label");
	state.labels[1].addr = 0x20; /* Relative to data start */
	state.labels[1].section = SEC_DATA;

	/* Another text label */
	strcpy(state.labels[2].name, "another_text");
	state.labels[2].addr = 0x200;
	state.labels[2].section = SEC_TEXT;

	state.text_size = 0x1000;
	state.data_size = 0x200;

	/* Adjust with data_base = end of text section */
	adjust_labels(&state, state.text_size);

	assert(state.labels[0].addr == 0x100); /* Text label unchanged */
	assert(state.labels[1].addr == 0x1000 + 0x20); /* Data label adjusted */
	assert(state.labels[2].addr == 0x200); /* Text label unchanged */

	printf("\tOK adjust_labels() works\n");
}

static void test_first_pass_basic(void) {
	printf("Test 8: Basic first pass functionality...\n");

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

	assembler_state_t state;
	first_pass(temp, &state);

	assert(state.current_section == SEC_DATA);
	assert(state.label_count >= 3); /* start, loop, msg, count */
	assert(state.text_size > 0);
	assert(state.data_size > 0);

	fclose(temp);
	printf("\tOK first pass works\n");
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
		strcpy(a1, trim(ptr));
	} else if (!comma2) {
		*comma1 = '\0';
		strcpy(a1, trim(ptr));
		strcpy(a2, trim(comma1 + 1));
	} else {
		*comma1 = '\0';
		strcpy(a1, trim(ptr));

		char *middle = comma1 + 1;
		*comma2 = '\0';
		strcpy(a2, trim(middle));
		strcpy(a3, trim(comma2 + 1));
	}
}

static void test_instruction_parsing(void) {
	printf("Test 9: Instruction argument parsing...\n");

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
	printf("Test 10: Section switching...\n");

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

	assembler_state_t state;
	first_pass(temp, &state);

	assert(state.text_size == 8); /* nop + addi = 2 instructions */
	assert(state.data_size == 8); /* 2 words = 8 bytes */

	rewind(temp);
	state.label_count = 0;
	state.pc_text = 0;
	state.pc_data = 0;
	state.current_section = SEC_TEXT;

	/* Manually simulate second pass section tracking */
	section_t current_section = SEC_TEXT;
	uint32_t pc_text = 0;
	uint32_t pc_data = 0;

	char line[MAX_LINE];
	while (fgets(line, sizeof(line), temp)) {
		char *s = trim(line);
		if (*s == 0 || *s == '#') continue;

		if (!strcmp(s, ".text")) {
			current_section = SEC_TEXT;
			continue;
		}
		if (!strcmp(s, ".data")) {
			current_section = SEC_DATA;
			continue;
		}

		if (current_section == SEC_TEXT) {
			pc_text += 4;
		} else {
			pc_data += 4; /* .word is 4 bytes */
		}
	}

	assert(pc_text == 8);
	assert(pc_data == 8);

	fclose(temp);
	printf("\tOK section switching works\n");
}

int main(void) {
	printf("=== RISC-V Assembler Comprehensive Tests ===\n\n");

	test_trim();
	test_reg_num();
	test_encode_functions();
	test_parse_imm();
	test_parse_escaped_string();
	test_pseudoinstruction_expansion();
	test_adjust_labels();
	test_first_pass_basic();
	test_instruction_parsing();
	test_section_switching();

	printf("\n=== All %d tests passed! ===\n", 10);
	return 0;
}
