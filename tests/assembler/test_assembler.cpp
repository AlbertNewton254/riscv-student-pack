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

static void test_call_ret_pseudoinstructions(void) {
	printf("Test 11: Call/Ret pseudoinstructions...\n");

	const char *assembly =
		".text\n"
		"main:\n"
		"	call func\n"
		"	j end\n"
		"func:\n"
		"	nop\n"
		"	ret\n"
		"end:\n"
		"	nop\n";

	FILE *in = tmpfile();
	FILE *out = tmpfile();
	fputs(assembly, in);
	rewind(in);

	Assembler assembler;
	assembler.first_pass(in);

	/* call (1 instr) + j (1 instr) + nop (1 instr) + ret (1 instr) + nop (1 instr) = 5 instructions */
	assert(assembler.get_text_size() == 20);

	assembler.adjust_labels(assembler.get_text_size());
	assembler.second_pass(in, out);

	/* Verify output was generated */
	fseek(out, 0, SEEK_END);
	long size = ftell(out);
	assert(size == 20);

	fclose(in);
	fclose(out);
	printf("\tOK call/ret pseudoinstructions work\n");
}

static void test_la_pseudoinstruction(void) {
	printf("Test 12: Load Address (la) pseudoinstruction...\n");

	const char *assembly =
		".text\n"
		"main:\n"
		"	la x1, data_label\n"
		"	nop\n"
		".data\n"
		"data_label:\n"
		"	.word 42\n";

	FILE *in = tmpfile();
	FILE *out = tmpfile();
	fputs(assembly, in);
	rewind(in);

	Assembler assembler;
	assembler.first_pass(in);

	/* la expands to auipc + addi (2 instr) + nop (1 instr) = 3 instructions = 12 bytes */
	assert(assembler.get_text_size() == 12);

	assembler.adjust_labels(assembler.get_text_size());
	assembler.second_pass(in, out);

	fclose(in);
	fclose(out);
	printf("\tOK la pseudoinstruction works\n");
}

static void test_custom_sections(void) {
	printf("Test 13: Custom section names (.text.startup, .rodata, .bss)...\n");

	/* Test 1: .text.startup subsection */
	const char *assembly1 =
		".section .text.startup\n"
		"	addi x1, x0, 1\n"
		"	addi x2, x0, 2\n";

	FILE *in1 = tmpfile();
	fputs(assembly1, in1);
	rewind(in1);

	Assembler assembler1;
	assembler1.first_pass(in1);

	/* .text.startup should be treated as text: 2 instructions = 8 bytes */
	assert(assembler1.get_text_size() == 8);
	fclose(in1);

	/* Test 2: .rodata section */
	const char *assembly2 =
		".section .rodata\n"
		"	.word 42\n"
		"	.word 100\n";

	FILE *in2 = tmpfile();
	fputs(assembly2, in2);
	rewind(in2);

	Assembler assembler2;
	assembler2.first_pass(in2);

	/* .rodata should be treated as data: 2 words = 8 bytes */
	assert(assembler2.get_data_size() == 8);
	fclose(in2);

	/* Test 3: .bss section */
	const char *assembly3 =
		".section .bss\n"
		"	.space 16\n";

	FILE *in3 = tmpfile();
	fputs(assembly3, in3);
	rewind(in3);

	Assembler assembler3;
	assembler3.first_pass(in3);

	/* .bss should be treated as data: 16 bytes */
	assert(assembler3.get_data_size() == 16);
	fclose(in3);

	printf("\tOK custom section names work\n");
}

static void test_data_directives(void) {
	printf("Test 14: Data directives (.byte, .half, .space)...\n");

	const char *assembly =
		".data\n"
		"byte_val:\n"
		"	.byte 0x12\n"
		"half_val:\n"
		"	.half 0x1234\n"
		"word_val:\n"
		"	.word 0x12345678\n"
		"space_val:\n"
		"	.space 10\n";

	FILE *in = tmpfile();
	fputs(assembly, in);
	rewind(in);

	Assembler assembler;
	assembler.first_pass(in);

	/* .byte (1) + .half (2) + .word (4) + .space (10) = 17 bytes */
	assert(assembler.get_data_size() == 17);

	fclose(in);
	printf("\tOK data directives work\n");
}

static void test_ascii_directives(void) {
	printf("Test 15: ASCII directives...\n");

	const char *assembly =
		".data\n"
		"str1:\n"
		"	.ascii \"Hello\"\n"
		"str2:\n"
		"	.asciiz \"World\"\n";

	FILE *in = tmpfile();
	FILE *out = tmpfile();
	fputs(assembly, in);
	rewind(in);

	Assembler assembler;
	assembler.first_pass(in);

	/* "Hello" (5 bytes) + "World\0" (6 bytes) = 11 bytes */
	assert(assembler.get_data_size() == 11);

	assembler.adjust_labels(0);  /* No text section */
	assembler.second_pass(in, out);

	/* Verify output */
	fseek(out, 0, SEEK_END);
	long size = ftell(out);
	assert(size == 11);

	/* Verify content */
	rewind(out);
	char buffer[12];
	size_t bytes_read = fread(buffer, 1, 11, out);
	assert(bytes_read == 11);
	assert(memcmp(buffer, "Hello", 5) == 0);
	assert(memcmp(buffer + 5, "World", 6) == 0);
	assert(buffer[10] == '\0');

	fclose(in);
	fclose(out);
	printf("\tOK ASCII directives work\n");
}

static void test_forward_backward_labels(void) {
	printf("Test 16: Forward and backward label references...\n");

	const char *assembly =
		".text\n"
		"start:\n"
		"	beq x1, x2, forward\n"
		"backward:\n"
		"	nop\n"
		"	j start\n"
		"forward:\n"
		"	beq x3, x4, backward\n"
		"	nop\n";

	FILE *in = tmpfile();
	FILE *out = tmpfile();
	fputs(assembly, in);
	rewind(in);

	Assembler assembler;
	assembler.first_pass(in);

	/* 5 instructions = 20 bytes */
	assert(assembler.get_text_size() == 20);

	assembler.adjust_labels(assembler.get_text_size());
	assembler.second_pass(in, out);

	/* Verify output was generated */
	fseek(out, 0, SEEK_END);
	long size = ftell(out);
	assert(size == 20);

	fclose(in);
	fclose(out);
	printf("\tOK forward/backward label references work\n");
}

static void test_multiple_data_items(void) {
	printf("Test 17: Multiple data items per directive...\n");

	const char *assembly =
		".data\n"
		"bytes:\n"
		"	.byte 1, 2, 3, 4, 5\n"
		"halfs:\n"
		"	.half 10, 20, 30\n"
		"words:\n"
		"	.word 100, 200, 300, 400\n";

	FILE *in = tmpfile();
	FILE *out = tmpfile();
	fputs(assembly, in);
	rewind(in);

	Assembler assembler;
	assembler.first_pass(in);

	/* .byte: 5 bytes, .half: 6 bytes, .word: 16 bytes = 27 bytes */
	assert(assembler.get_data_size() == 27);

	assembler.adjust_labels(0);
	assembler.second_pass(in, out);

	/* Verify output */
	fseek(out, 0, SEEK_END);
	long size = ftell(out);
	assert(size == 27);

	/* Verify some values */
	rewind(out);
	uint8_t bytes[5];
	size_t bytes_read2 = fread(bytes, 1, 5, out);
	assert(bytes_read2 == 5);
	assert(bytes[0] == 1 && bytes[1] == 2 && bytes[4] == 5);

	fclose(in);
	fclose(out);
	printf("\tOK multiple data items per directive work\n");
}

static void test_m_extension_mul() {
	printf("Test 18: M Extension - MUL instructions...\n");

	const char *assembly =
		".text\n"
		"	mul x1, x2, x3\n"
		"	mulh x4, x5, x6\n"
		"	mulhsu x7, x8, x9\n"
		"	mulhu x10, x11, x12\n";

	FILE *in = tmpfile();
	FILE *out = tmpfile();
	fputs(assembly, in);
	rewind(in);

	Assembler assembler;
	assembler.first_pass(in);
	assembler.second_pass(in, out);

	/* Verify output size - 4 instructions = 16 bytes */
	fseek(out, 0, SEEK_END);
	long size = ftell(out);
	assert(size == 16);

	/* Verify instruction encodings */
	rewind(out);
	uint32_t instrs[4];
	size_t count = fread(instrs, sizeof(uint32_t), 4, out);
	assert(count == 4);

	/* mul x1, x2, x3: funct7=0x01, rs2=x3, rs1=x2, funct3=0x0, rd=x1, opcode=0x33 */
	assert(instrs[0] == 0x023100B3);

	/* mulh x4, x5, x6: funct7=0x01, rs2=x6, rs1=x5, funct3=0x1, rd=x4, opcode=0x33 */
	assert(instrs[1] == 0x02629233);

	/* mulhsu x7, x8, x9: funct7=0x01, rs2=x9, rs1=x8, funct3=0x2, rd=x7, opcode=0x33 */
	assert(instrs[2] == 0x029423B3);

	/* mulhu x10, x11, x12: funct7=0x01, rs2=x12, rs1=x11, funct3=0x3, rd=x10, opcode=0x33 */
	assert(instrs[3] == 0x02C5B533);

	fclose(in);
	fclose(out);
	printf("\tOK M Extension MUL instructions work\n");
}

static void test_m_extension_div() {
	printf("Test 19: M Extension - DIV instructions...\n");

	const char *assembly =
		".text\n"
		"	div x1, x2, x3\n"
		"	divu x4, x5, x6\n"
		"	rem x7, x8, x9\n"
		"	remu x10, x11, x12\n";

	FILE *in = tmpfile();
	FILE *out = tmpfile();
	fputs(assembly, in);
	rewind(in);

	Assembler assembler;
	assembler.first_pass(in);
	assembler.second_pass(in, out);

	/* Verify output size - 4 instructions = 16 bytes */
	fseek(out, 0, SEEK_END);
	long size = ftell(out);
	assert(size == 16);

	/* Verify instruction encodings */
	rewind(out);
	uint32_t instrs[4];
	size_t count = fread(instrs, sizeof(uint32_t), 4, out);
	assert(count == 4);

	/* div x1, x2, x3: funct7=0x01, rs2=x3, rs1=x2, funct3=0x4, rd=x1, opcode=0x33 */
	assert(instrs[0] == 0x023140B3);

	/* divu x4, x5, x6: funct7=0x01, rs2=x6, rs1=x5, funct3=0x5, rd=x4, opcode=0x33 */
	assert(instrs[1] == 0x0262D233);

	/* rem x7, x8, x9: funct7=0x01, rs2=x9, rs1=x8, funct3=0x6, rd=x7, opcode=0x33 */
	assert(instrs[2] == 0x029463B3);

	/* remu x10, x11, x12: funct7=0x01, rs2=x12, rs1=x11, funct3=0x7, rd=x10, opcode=0x33 */
	assert(instrs[3] == 0x02C5F533);

	fclose(in);
	fclose(out);
	printf("\tOK M Extension DIV instructions work\n");
}

static void test_m_extension_with_abi_names() {
	printf("Test 20: M Extension - Using ABI register names...\n");

	const char *assembly =
		".text\n"
		"	mul a0, a1, a2\n"
		"	div t0, t1, t2\n"
		"	rem s0, s1, s2\n"
		"	mulh ra, sp, gp\n";

	FILE *in = tmpfile();
	FILE *out = tmpfile();
	fputs(assembly, in);
	rewind(in);

	Assembler assembler;
	assembler.first_pass(in);
	assembler.second_pass(in, out);

	/* Verify output size - 4 instructions = 16 bytes */
	fseek(out, 0, SEEK_END);
	long size = ftell(out);
	assert(size == 16);

	/* Verify instruction encodings */
	rewind(out);
	uint32_t instrs[4];
	size_t count = fread(instrs, sizeof(uint32_t), 4, out);
	assert(count == 4);

	/* mul a0, a1, a2 -> mul x10, x11, x12 */
	assert(instrs[0] == 0x02C58533);

	/* div t0, t1, t2 -> div x5, x6, x7 */
	assert(instrs[1] == 0x027342B3);

	/* rem s0, s1, s2 -> rem x8, x9, x18 */
	assert(instrs[2] == 0x0324E433);

	/* mulh ra, sp, gp -> mulh x1, x2, x3 */
	assert(instrs[3] == 0x023110B3);

	fclose(in);
	fclose(out);
	printf("\tOK M Extension with ABI names works\n");
}

static void test_m_extension_mixed_program() {
	printf("Test 21: M Extension - Mixed program with labels...\n");

	const char *assembly =
		".text\n"
		"main:\n"
		"	addi a0, zero, 6\n"
		"	addi a1, zero, 7\n"
		"	mul a2, a0, a1\n"
		"	div a3, a2, a1\n"
		"	rem a4, a2, a0\n"
		"	ret\n";

	FILE *in = tmpfile();
	FILE *out = tmpfile();
	fputs(assembly, in);
	rewind(in);

	Assembler assembler;
	assembler.first_pass(in);
	assembler.adjust_labels(assembler.get_text_size());
	assembler.second_pass(in, out);

	/* Verify output size - 6 instructions = 24 bytes */
	fseek(out, 0, SEEK_END);
	long size = ftell(out);
	assert(size == 24);

	/* Verify we can read all instructions */
	rewind(out);
	uint32_t instrs[6];
	size_t count = fread(instrs, sizeof(uint32_t), 6, out);
	assert(count == 6);

	/* Check the M extension instructions are encoded correctly */
	/* mul a2, a0, a1 -> mul x12, x10, x11 */
	assert(instrs[2] == 0x02B50633);

	/* div a3, a2, a1 -> div x13, x12, x11 */
	assert(instrs[3] == 0x02B646B3);

	/* rem a4, a2, a0 -> rem x14, x12, x10 */
	assert(instrs[4] == 0x02A66733);

	fclose(in);
	fclose(out);
	printf("\tOK M Extension mixed program works\n");
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
	test_call_ret_pseudoinstructions();
	test_la_pseudoinstruction();
	test_custom_sections();
	test_data_directives();
	test_ascii_directives();
	test_forward_backward_labels();
	test_multiple_data_items();
	test_m_extension_mul();
	test_m_extension_div();
	test_m_extension_with_abi_names();
	test_m_extension_mixed_program();

	printf("\n=== All %d tests passed! ===\n", 21);
	return 0;
}