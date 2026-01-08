# Test Infrastructure

Comprehensive test suite with unit tests for assembler and emulator, plus integration tests for end-to-end validation.

## Brief Description

The test suite provides comprehensive coverage of both the assembler and emulator components. It includes unit tests for individual functionality (label resolution, instruction encoding, CPU operations, memory management) and integration tests that verify complete workflows from assembly to execution. All tests can be run together or individually.

## Folder Structure

```
tests/
├── Makefile                    Build configuration
├── README.md                   This file
├── all_tests.sh                Master test runner script
├── unit_tests.sh               Unit tests only script
├── integration_tests.sh        Integration tests only script
├── assembler/
│   └── test_assembler.cpp      10 assembler unit tests
├── emulator/
│   └── test_emulator.cpp       12 emulator unit tests
└── integration/
    └── test_integration.cpp    End-to-end tests
```

## Features

- 10 assembler unit tests covering all instruction formats and directives
- 12 emulator unit tests covering CPU, memory, and instruction execution
- Integration tests for end-to-end workflows
- Multiple test runners: make, shell scripts, and root Makefile
- Parallel build support with make -j
- Exit codes for CI/CD integration (0 = success, 1 = failure)

## Documentation

### Unit Tests

#### Assembler Tests (10 suites)

1. Label Resolution - Forward and backward references
2. Instruction Encoding - All formats (R, I, S, B, U, J)
3. Pseudoinstruction Expansion - li, la, mv, call, ret, j, nop
4. Data Directives - .byte, .half, .word, .ascii, .asciiz, .space
5. Section Management - .text, .data, .rodata, .bss, .section
6. Symbol Table - Global and local labels
7. Address Calculation - Text and data sections
8. Immediate Handling - Sign extension, truncation
9. Error Detection - Invalid instructions, bad labels
10. GAS Compatibility - GNU Assembler section formats

Run: `make run_test_assembler` or `./test_assembler`

#### Emulator Tests (12 suites)

1. CPU Initialization - Registers, PC, state
2. Memory Operations - Load/store, alignment, bounds checking
3. Sign Extension - 8/16-bit loads, sign propagation
4. Instruction Decoding - All RV32I formats
5. Register Operations - Read/write, x0 special case
6. ALU Operations - add, sub, logical, shift instructions
7. Load Instructions - lw, lh, lhu, lb, lbu
8. Store Instructions - sw, sh, sb with various alignments
9. Branch Instructions - beq, bne, blt, bge, bltu, bgeu
10. Jump Instructions - jal, jalr (call/return)
11. System Calls - exit, write, read, brk
12. Complete Execution - Multi-instruction programs

Run: `make run_test_emulator` or `./test_emulator`

### Integration Tests

Located in integration/test_integration.cpp:

End-to-end tests that:
1. Write assembly source file
2. Run assembler to generate binary
3. Run emulator on binary
4. Verify output and results

Test cases:
- Hello World - Basic string output
- Arithmetic - Multi-instruction computation
- Control Flow - Loops and branches
- Memory Access - Load/store operations
- Function Calls - Call/return with stack
- System Calls - I/O operations

Run: `make run_test_integration` or `./test_integration`

### Running Tests

#### From tests/ Directory

```bash
cd tests

# All tests
make test                      # Run all unit + integration

# By type
make run_test_assembler        # Assembler only
make run_test_emulator         # Emulator only
make run_test_integration      # Integration only

# Build without running
make test_assembler
make test_emulator
make test_integration

# Clean
make clean                     # Remove all test artifacts
```

### From Root Directory

```bash
cd ..

# All tests
make test                      # All tests
make test-all                  # Alias
make unit-tests                # Unit tests only
make test-assembler            # Assembler only
make test-emulator             # Emulator only
make test-integration          # Integration only
make integration-tests         # Full suite with cleanup

# Run shell scripts
./tests/all_tests.sh          # All tests
./tests/unit_tests.sh         # Unit tests
./tests/integration_tests.sh  # Integration
```

#### Using make -j for Parallel Builds

```bash
make -j4 test                  # Build and run in parallel
make -j$(nproc) test          # Use all CPU cores
```

### Test Output

Successful tests show:
```
===== Assembler Tests =====
[PASS] Label Resolution
[PASS] Instruction Encoding
... (more tests)

===== Emulator Tests =====
[PASS] CPU Initialization
[PASS] Memory Operations
... (more tests)

===== Integration Tests =====
[PASS] Hello World
[PASS] Arithmetic
... (more tests)

All tests passed!
```

Failed tests show:
```
[FAIL] Test Name
  Error: assertion failed at file:line
  Details: ...
```

Exit code: 0 on success, 1 on failure

### Writing New Tests

#### Adding Assembler Tests

Edit assembler/test_assembler.cpp:

```cpp
// Add to existing test function or create new
void test_new_feature() {
    Assembler asm;
    // ... setup
    assert(asm.some_method() == expected);
    cout << "[PASS] New Feature Test\n";
}
```

#### Adding Emulator Tests

Edit emulator/test_emulator.cpp:

```cpp
void test_new_instruction() {
    CPU cpu;
    Memory mem;
    // ... setup
    cpu.step();
    assert(cpu.get_register(1) == expected);
    cout << "[PASS] New Instruction Test\n";
}
```

#### Adding Integration Tests

Edit integration/test_integration.cpp:

```cpp
void test_new_program() {
    // Write assembly file
    ofstream src("test.s");
    src << ".text\n_start:\n  ...";
    src.close();

    // Assemble
    system("../assembler/riscv_assembler test.s test.bin");

    // Run
    system("../emulator/riscv_emulator test.bin > output.txt");

    // Verify
    // ... check output.txt
}
```

### Build System

#### Makefile Targets (from tests/)

```
test                  - Run all tests (unit + integration)
run_test_assembler    - Run assembler tests
run_test_emulator     - Run emulator tests
run_test_integration  - Run integration tests
test_assembler        - Build assembler test executable
test_emulator         - Build emulator test executable
test_integration      - Build integration test executable
clean                 - Remove all test artifacts
format                - Format test code
help                  - Show targets
```

#### Compilation Flags

Same as main project:
- -O2 -Wall -Wextra -Werror -std=c++17 (default)
- -O0 -g -DDEBUG (debug)
- -O3 -DNDEBUG (release)

Include paths: -I../assembler/include -I../emulator/include

### Test Coverage

#### Assembler Coverage

- Instruction formats: R, I, S, B, U, J (check)
- Pseudoinstructions: li, la, mv, call, ret, j, nop (check)
- Data directives: .byte, .half, .word, .ascii, .asciiz (check)
- Sections: .text, .data, .rodata, .bss, .section (check)
- Label resolution: forward/backward refs (check)
- Symbol table management (check)
- Address calculation (check)
- Error handling (check)

#### Emulator Coverage

- Instruction execution: All 32 RV32I base (check)
- Register operations: Read, write, x0 handling (check)
- Memory operations: Load/store with alignment (check)
- Control flow: Branches, jumps, call/return (check)
- Syscalls: exit, write, read, brk (check)
- Error detection: Out of bounds, alignment (check)
- State management: CPU initialization (check)

#### Integration Coverage

- Complete programs: hello, loops, functions (check)
- Multi-instruction sequences (check)
- Data section usage (check)
- System call integration (check)
- Error handling in full workflow (check)

### Continuous Integration

#### Running in CI/CD

```bash
# GitHub Actions example
- name: Test
  run: |
    cd riscv-student-pack
    make clean-all
    make test
```

All tests return exit code 0 on success, 1 on failure.

### Troubleshooting

#### Test Failures

**Assembler test fails:**
1. Check syntax in test_assembler.cpp
2. Verify assembler headers are available
3. Clean and rebuild: `make clean && make test_assembler`

**Emulator test fails:**
1. Check CPU/Memory class implementations
2. Verify instruction encodings
3. Review error output for specific assertion

**Integration test fails:**
1. Verify assembler and emulator built correctly
2. Check assembly file syntax in test
3. Run individual components separately

#### Build Issues

```bash
# Clean everything and rebuild
make clean
make test_assembler

# Verbose output (if make verbose mode available)
make V=1 test

# Check dependencies
ls ../assembler/riscv_assembler  # Should exist
ls ../emulator/riscv_emulator    # Should exist
```

### Test Scripts

#### all_tests.sh

Master test runner:
```bash
./all_tests.sh
```

Runs:
1. Unit tests (assembler + emulator)
2. Integration tests
3. Cleanup
4. Summary report

#### unit_tests.sh

Unit tests only:
```bash
./unit_tests.sh
```

#### integration_tests.sh

Integration tests only:
```bash
./integration_tests.sh
```

### Performance Testing

For performance-sensitive changes:

```bash
# Build release version
make release

# Time test execution
time make test

# Profile emulator
cd ../emulator
time ./riscv_emulator /path/to/large/program.bin
```

## See Also

- [Main README](../README.md) - Project overview
- [Assembler](../assembler/README.md) - Assembly details
- [Emulator](../emulator/README.md) - Execution details
- [QUICKSTART](../QUICKSTART.md) - Usage guide
