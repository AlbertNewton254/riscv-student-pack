# RISC-V Student Pack

Educational toolkit for learning RISC-V: assembler and emulator in modern C++17.

## Brief Description

The RISC-V Student Pack provides two complementary tools for learning assembly language programming:

- **Assembler**: Two-pass assembler translating RV32I assembly to flat binary executables. Implements symbol resolution, instruction encoding, and pseudoinstruction expansion. Supports M extension for multiplication and division.
- **Emulator**: Instruction simulator executing RV32I binaries with full base integer instruction set, M extension (multiply/divide), and Linux ABI syscalls.

Both tools are written in modern C++17 with object-oriented design for clarity, safety, and extensibility.

For quick start, see [QUICKSTART.md](QUICKSTART.md).

## Project Structure

```
riscv-student-pack/
├── assembler/               Two-pass RV32I assembler
│   ├── Makefile
│   ├── README.md
│   ├── riscv_assembler      Executable
│   ├── include/
│   │   └── assembler.hpp
│   └── src/
│       ├── main.cpp
│       ├── constructor.cpp
│       ├── first_pass.cpp
│       ├── second_pass.cpp
│       ├── encode.cpp
│       ├── expand_pseudoinstruction.cpp
│       ├── adjust_labels.cpp
│       └── utils.cpp
├── emulator/                RV32I instruction simulator
│   ├── Makefile
│   ├── README.md
│   ├── riscv_emulator       Executable
│   ├── include/
│   │   ├── cpu.hpp
│   │   ├── memory.hpp
│   │   └── instructions.hpp
│   └── src/
│       ├── main.cpp
│       ├── cpu.cpp
│       ├── memory.cpp
│       └── instructions.cpp
├── tests/                   Test infrastructure
│   ├── Makefile
│   ├── README.md
│   ├── all_tests.sh
│   ├── unit_tests.sh
│   ├── integration_tests.sh
│   ├── assembler/
│   │   └── test_assembler.cpp
│   ├── emulator/
│   │   └── test_emulator.cpp
│   └── integration/
│       └── test_integration.cpp
├── Makefile                 Root build configuration
├── README.md                This file
├── QUICKSTART.md            Quick start guide
└── LICENSE                  MIT License
```

## Features

### Assembler

The assembler translates RISC-V assembly language to machine code:

- All 32 RV32I base instructions (arithmetic, logical, shifts, comparisons, branches, jumps, loads, stores, upper immediates, system)
- 7 pseudoinstructions: li, la, mv, nop, call, ret, j
- GNU-compatible section directives (.text, .data, .rodata, .bss, .section)
- Data directives (.ascii, .asciiz, .byte, .half, .word, .space)
- Forward and backward label resolution
- Debug mode with detailed output
- Two-pass design: symbol resolution followed by code generation

See [assembler/README.md](assembler/README.md) for detailed documentation.

### Emulator

The emulator executes binary programs with full instruction simulation:

- Full RV32I fetch-decode-execute pipeline
- 32 registers with standard ABI names
- 16 MiB configurable memory with bounds checking
- Linux ABI syscalls: exit, read, write, openat, close, fstat, brk
- Register dumps and stack traces on errors
- Debug mode with instruction tracing
- Alignment validation and error detection

See [emulator/README.md](emulator/README.md) for detailed documentation.

### Testing

Comprehensive test suite covering all components:

- 10 assembler unit tests (label resolution, instruction encoding, pseudoinstructions, data directives, sections)
- 12 emulator unit tests (CPU, memory, instructions, branches, jumps, syscalls)
- Integration tests for end-to-end workflows
- Test runners at all levels with shell scripts
- Parallel build support with make -j

See [tests/README.md](tests/README.md) for detailed documentation.

## Documentation

### Assembler

#### Instruction Set

The assembler supports all 32 RV32I base instructions:

**RV32I Base (32):**
- Arithmetic: add, sub, addi
- Logical: and, or, xor, andi, ori, xori
- Shifts: sll, srl, sra, slli, srli, srai
- Comparison: slt, sltu, slti, sltiu
- Branches: beq, bne, blt, bge, bltu, bgeu
- Jumps: jal, jalr
- Loads: lb, lh, lw, lbu, lhu
- Stores: sb, sh, sw
- Upper Immediate: lui, auipc
- System: ecall, ebreak

**M Extension (8):**
- Multiply: mul, mulh, mulhsu, mulhu
- Divide/Remainder: div, divu, rem, remu

#### Pseudoinstructions

Common pseudoinstructions expand to base RV32I:

| Pseudo | Expands To | Purpose |
|--------|-----------|---------|
| li rd, imm | lui + addi or addi | Load immediate |
| la rd, label | auipc + addi | Load address |
| mv rd, rs | addi rd, rs, 0 | Copy register |
| nop | addi x0, x0, 0 | No operation |
| call label | jal ra, label | Call function |
| ret | jalr x0, ra, 0 | Return from function |
| j label | jal x0, label | Unconditional jump |

#### Directives

Section directives organize code and data:

- .text - Code section (default)
- .data - Initialized data
- .rodata - Read-only data
- .bss - Uninitialized data
- .section <name> - Arbitrary section

Data directives reserve and initialize memory:

- .byte - 8-bit values
- .half - 16-bit values
- .word - 32-bit values
- .ascii - String without null terminator
- .asciiz - Null-terminated string
- .space - Reserve bytes

#### Usage

```bash
./riscv_assembler input.s output.bin
./riscv_assembler --debug input.s output.bin
```

### Emulator

#### Instruction Execution

The emulator implements a fetch-decode-execute pipeline:

1. Fetch: Load instruction from memory at program counter
2. Decode: Parse instruction format and extract operands
3. Execute: Perform operation and update processor state
4. Repeat until program exit or error

#### Memory Layout

```
0x00000000 +----------------+
           | .text          | Code section
           | .data          | Initialized data
           | .rodata        | Read-only data
           +----------------+
           | Heap           | Dynamic allocation
           | (grows up)     |
           |----------------|
0x80000000 | Stack          | Function calls, locals
           | (grows down)   |
0xFFFFFFFF +----------------+
```

#### Syscalls

Linux ABI syscalls for I/O and process control:

| Number | Name | Arguments | Returns | Purpose |
|--------|------|-----------|---------|---------|
| 93 | exit | a0=code | - | Exit with status |
| 63 | read | a0=fd, a1=buf, a2=len | count | Read from file |
| 64 | write | a0=fd, a1=buf, a2=len | count | Write to file |
| 56 | openat | a0=dirfd, a1=path, a2=flags | fd | Open file |
| 57 | close | a0=fd | 0 | Close file |
| 80 | fstat | a0=fd, a1=buf | 0 | Query file |
| 214 | brk | a0=addr | new_brk | Heap control |

#### Usage

```bash
./riscv_emulator program.bin
./riscv_emulator --debug program.bin
```

### Testing

#### Test Types

- Unit tests: Individual component testing (assembler and emulator)
- Integration tests: End-to-end workflows from assembly to execution

#### Running Tests

```bash
make test              # All tests
make test-assembler    # Assembler only
make test-emulator     # Emulator only
make test-integration  # Integration tests
```

## Build System

Build both tools from the root directory:

```bash
make                  # Build assembler and emulator
make assembler        # Build assembler only
make emulator         # Build emulator only
make test            # Run all tests
make clean           # Remove object files
make clean-all       # Remove all build artifacts
make help            # Show all targets
```

Each component (assembler, emulator, tests) can be built independently with its own Makefile.

## Design Philosophy

This project demonstrates professional software engineering in C++:

### Architecture

- Object-Oriented: CPU, Memory, Instruction, Assembler classes with clear responsibilities
- Encapsulation: Private state, public interfaces, controlled access
- RAII: Constructors/destructors handle resource management automatically
- Type Safety: std::array, std::vector, std::map instead of raw pointers

### Code Quality

- Modern C++17: Range-based loops, const correctness, move semantics
- Defensive: Bounds checking, alignment validation, comprehensive error handling
- Testable: 22+ unit tests with full coverage
- Debuggable: Optional debug mode for tracing execution

### Learning Focus

- Clear separation of concerns
- Readable code with explanatory comments
- Realistic instruction set (RV32I is baseline for all RISC-V)
- Practical syscall implementation for real-world integration

## Prerequisites

- G++ with C++17 support (GCC 7.0 or later)
- GNU Make
- Linux operating system (for syscalls)

## Getting Started

1. Read [QUICKSTART.md](QUICKSTART.md) for immediate usage (5 minutes)
2. Study [assembler/README.md](assembler/README.md) for assembly details
3. Study [emulator/README.md](emulator/README.md) for execution details
4. Review [tests/README.md](tests/README.md) for test examples
5. Run `make help` for all build targets

## Contributing

Contributions welcome! Areas of interest:

- Test cases and example programs
- Documentation and tutorials
- Bug reports with reproducible cases
- Performance improvements

Open an issue before making major changes.

## License

MIT License. See [LICENSE](./LICENSE) for details.

Free to use, modify, and extend for educational and other purposes.