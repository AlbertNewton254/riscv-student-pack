#include "../include/assembler.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

int main() {
    printf("=== RISC-V Assembler Basic Tests ===\n\n");

    // Test 1: trim() function
    printf("Test 1: trim() function...\n");
    char test1[] = "  hello  ";
    char *result = trim(test1);
    assert(strcmp(result, "hello") == 0);
    printf("  ✓ trim() works\n");

    // Test 2: reg_num() function
    printf("Test 2: reg_num() function...\n");
    assert(reg_num("x0") == 0);
    assert(reg_num("x31") == 31);
    assert(reg_num("a0") == 10);
    assert(reg_num("zero") == 0);
    printf("  ✓ reg_num() works\n");

    // Test 3: encode_r() function
    printf("Test 3: encode_r() function...\n");
    uint32_t instr = encode_r(0x00, 3, 2, 0x0, 1, 0x33);
    // add x1, x2, x3 = 0x003100B3
    assert(instr == 0x003100B3);
    printf("  ✓ encode_r() works\n");

    // Test 4: encode_i() function
    printf("Test 4: encode_i() function...\n");
    instr = encode_i(42, 2, 0x0, 1, 0x13);
    // addi x1, x2, 42 = 0x02A10093
    assert(instr == 0x02A10093);
    printf("  ✓ encode_i() works\n");

    // Test 5: parse_imm() function
    printf("Test 5: parse_imm() function...\n");
    assembler_state_t state = {0};
    // Test with NULL state (first pass)
    assert(parse_imm(NULL, "42") == 42);
    assert(parse_imm(NULL, "0xFF") == 0xFF);
    assert(parse_imm(NULL, "-10") == -10);
    printf("  ✓ parse_imm() works\n");

    printf("\n=== All basic assembler tests passed! ===\n");
    return 0;
}
