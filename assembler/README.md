# RISC-V Assembler

Two-pass assembler translating RISC-V assembly language (RV32I) to flat binary executables.

## Brief Description

The assembler reads assembly source files and produces machine code binaries compatible with the RISC-V emulator. It implements a two-pass design: the first pass resolves all labels and symbols, and the second pass encodes instructions with final addresses. The assembler supports all 32 RV32I base instructions, 7 pseudoinstructions, and GNU-compatible section directives.

## Folder Structure

```
assembler/
|-- Makefile                 Build configuration
|-- README.md                This file
|-- riscv_assembler          Executable
|-- include/
|   '-- assembler.hpp        Main assembler class
'-- src/
    |-- main.cpp             Entry point and CLI
    |-- constructor.cpp      Assembler initialization
    |-- first_pass.cpp       Symbol table building
    |-- second_pass.cpp      Instruction encoding
    |-- encode.cpp           Instruction format encoding
    |-- expand_pseudoinstruction.cpp
    |-- adjust_labels.cpp    Label address calculation
    '-- utils.cpp            Utility functions
```

## Features

- All 32 RV32I base instructions
- 7 pseudoinstructions (li, la, mv, nop, call, ret, j)
- GNU-compatible section directives (.text, .data, .rodata, .bss, .section)
- Data directives (.ascii, .asciiz, .byte, .half, .word, .space)
- Forward and backward label resolution
- Two-pass design for accurate symbol resolution
- Debug mode with detailed output
- Bounds checking and error detection

## Documentation

### Architecture

#### Two-Pass Design

The assembler uses a two-pass approach for accurate assembly:

**Pass 1 (Symbol Resolution)**
- Parse all lines and build symbol table
- Calculate memory addresses for labels
- Expand pseudoinstructions to determine instruction count
- Detect forward references and scope issues

**Pass 2 (Code Generation)**
- Encode instructions with resolved symbol addresses
- Generate binary output file
- Handle all relocations (label references)
- Report any remaining errors

#### Object Model

**Assembler Class** (include/assembler.hpp)
- Central object managing the assembly process
- Maintains symbol table: std::map<std::string, uint32_t>
- Tracks current section and address
- Methods for each assembly phase

**Instruction Encoder** (encode.cpp)
- Static encoding methods for each instruction format
- RV32I instruction formats: R, I, S, B, U, J
- Immediate value handling and validation

**Pseudoinstruction Expansion** (expand_pseudoinstruction.cpp)
- Expands high-level instructions to RV32I
- Handles register allocation for expansion

### Instruction Set

#### All 32 RV32I Base Instructions

| Type | Instructions |
|------|-------------|
| Arithmetic | add, sub, addi |
| Logical | and, or, xor, andi, ori, xori |
| Shifts | sll, srl, sra, slli, srli, srai |
| Comparison | slt, sltu, slti, sltiu |
| Branches | beq, bne, blt, bge, bltu, bgeu |
| Jumps | jal, jalr |
| Loads | lb, lh, lw, lbu, lhu |
| Stores | sb, sh, sw |
| Upper Immediate | lui, auipc |
| System | ecall, ebreak |

#### Pseudoinstructions

Common pseudoinstructions expand to RV32I:

| Pseudo | Expands To | Purpose |
|--------|-----------|---------|
| li rd, imm | lui + addi or addi | Load immediate |
| la rd, label | auipc + addi | Load address |
| mv rd, rs | addi rd, rs, 0 | Copy register |
| nop | addi x0, x0, 0 | No operation |
| call label | jal ra, label | Call function |
| ret | jalr x0, ra, 0 | Return from function |
| j label | jal x0, label | Unconditional jump |

### Directives

#### Section Directives

Organize code and data into sections:

- .text - Code section (default)
- .data - Initialized data
- .rodata - Read-only data
- .bss - Uninitialized data
- .section <name> - Arbitrary section (GAS-compatible)

Example with custom sections:

```assembly
.section .text.startup
_start:
    li a0, 0
    li a7, 93
    ecall

.section .text
main:
    nop
```

#### Data Directives

Reserve and initialize memory:

| Directive | Usage | Example |
|-----------|-------|---------|
| .byte | 8-bit values | .byte 42, 0xFF |
| .half | 16-bit values | .half 1000, -500 |
| .word | 32-bit values | .word 0x12345678 |
| .ascii | String (no null) | .ascii "hello" |
| .asciiz | Null-terminated string | .asciiz "world" |
| .space | Reserve bytes | .space 100 |

#### Labels

Any identifier followed by colon:

```assembly
loop:
    addi x1, x1, 1
    bne x1, x2, loop
```

Labels can be:
- At start of line
- Followed by instruction on same line
- Referenced in branches/jumps (forward or backward)

### Usage

#### Basic Assembly

```bash
./riscv_assembler input.s output.bin
```

#### Debug Mode

```bash
./riscv_assembler --debug input.s output.bin
```

Debug output shows:
- Each line processed
- Pseudoinstruction expansions
- Symbol table contents
- Address calculations

#### Common Errors

| Error | Cause | Fix |
|-------|-------|-----|
| Undefined label | Reference before definition | Check label spelling |
| Invalid immediate | Constant out of range | Use smaller value or lui+addi |
| Format error | Wrong instruction syntax | Check instruction format |
| Section conflict | Bad section directive | Use .text, .data, .rodata, or .section |

### Implementation Details

#### Source Files

- main.cpp - Entry point and argument parsing
- constructor.cpp - Assembler initialization
- first_pass.cpp - Symbol table building
- second_pass.cpp - Instruction encoding
- encode.cpp - Instruction format encoding
- expand_pseudoinstruction.cpp - Pseudoinstruction expansion
- adjust_labels.cpp - Label address calculation
- utils.cpp - Utility functions (parsing, formatting)

#### Symbol Table

```cpp
std::map<std::string, uint32_t> symbol_table;  // label -> address
```

Labels are resolved in first pass. Forward references are stored for second pass.

#### Output Format

Binary file containing:
- .text section (code) - starts at 0x00000000
- .data section (initialized data) - follows .text
- .rodata section (read-only) - follows .data
- .bss section (uninitialized) - followed by heap

Each section is contiguous with natural alignment.

### Build

#### Targets

```bash
make              Build assembler executable
make run PROGRAM=test.s    Build and run (assembles test.s)
make clean        Remove object files
make clean-all    Remove everything
make test         Run unit tests (via root Makefile)
make format       Format code with clang-format
make analyze      Static analysis with cppcheck
make debug        Build with debug symbols
make release      Build optimized version
```

#### Compilation

- Standard: -O2 -Wall -Wextra -Werror -std=c++17
- Debug: -O0 -g -DDEBUG
- Release: -O3 -DNDEBUG

Requires: G++ 7.0+ (C++17 support)

### Testing

Unit tests cover:
- Label resolution (forward/backward)
- All instruction formats (R, I, S, B, U, J)
- Data directives
- Pseudoinstruction expansion
- Symbol table management

Run: `make test` (from root) or `cd ../tests && make run_test_assembler`

### Examples

#### Simple Loop

```assembly
.text
_start:
    li x1, 0        # Counter
    li x2, 10       # Limit

loop:
    addi x1, x1, 1  # Increment
    bne x1, x2, loop    # Loop if != 10

    # Exit
    li a7, 93
    ecall
```

#### With Data

```assembly
.text
_start:
    la a1, message  # Load message address
    li a2, 13       # Length
    li a0, 1        # stdout
    li a7, 64       # write syscall
    ecall

    li a0, 0        # Exit code
    li a7, 93       # exit syscall
    ecall

.data
message:
    .asciiz "Hello, RISC-V!\n"
```

### Integration with Emulator

Assembler output (binary) runs in emulator:

```bash
# Assemble
./riscv_assembler hello.s hello.bin

# Run
../emulator/riscv_emulator hello.bin
```

The binary format is a simple memory image:
- Raw bytes in execution order
- Emulator loads at address 0x00000000 (default)
- Sections concatenated without metadata

## See Also

- [Main README](../README.md) - Project overview
- [QUICKSTART](../QUICKSTART.md) - Usage guide
- [Emulator](../emulator/README.md) - Binary execution
- [Tests](../tests/README.md) - Test infrastructure