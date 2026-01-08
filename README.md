# RISC-V Student Pack

Educational toolkit for learning RISC-V: assembler and emulator in modern C++.

## Overview

Two complementary tools for RISC-V assembly programming:

**Assembler**: Two-pass assembler translating RV32I assembly to binaries.
First pass resolves labels, second pass encodes instructions.

**Emulator**: Instruction simulator executing RV32I binaries.
Implements base integer instructions with Linux ABI syscalls.

Both written in modern C++ (C++17) with object-oriented design for clarity, safety, and extensibility.

---

## Design Choices

### Object-Oriented Architecture

**Encapsulation**: CPU, Memory, Instruction, and Assembler classes with private state.
Clear interfaces, controlled access through public methods.

**RAII Resource Management**: Constructors/destructors handle initialization and cleanup.
Automatic resource safety, no manual memory management.

**Type-Safe Containers**:
- `std::array<uint32_t, 32>` for CPU registers (fixed size, bounds-checked)
- `std::vector` for dynamic label storage and memory
- `std::map` for symbol table (efficient lookups)

No arbitrary limits, compile-time safety where possible.

**Modern C++ Practices**:
- Range-based for loops for clarity
- Boolean return types instead of error codes
- Method chaining where appropriate
- Const correctness throughout

### Assembler Architecture

**Two-Pass Design**:
- Pass 1: Build symbol table with `std::map<std::string, uint32_t>`, calculate addresses
- Pass 2: Encode instructions with resolved symbols

**Assembler Class**: Central object managing the assembly process.
Encapsulates symbol table, current section, address tracking.

**Binary Output**: Flat binaries (not ELF) for simplicity.
What you see is what executes.

**Pseudoinstructions**: `li`, `la`, `mv`, `nop` expand to RV32I.
Idiomatic assembly, pure execution.

**Sections**: `.text` and `.data` with automatic address calculation.

### Emulator Architecture

**CPU Class**: Encapsulates registers (`std::array<uint32_t, 32>`), PC, and execution state.
Methods for register access, instruction fetch/decode/execute.

**Memory Class**: Manages RAM with `std::vector<uint8_t>`.
Bounds checking, alignment validation, separate read/write methods.

**Instruction Class**: Decodes and represents RISC-V instructions.
Type-safe format detection, immediate value extraction.

**Fetch-Decode-Execute**: Clean separation with dedicated methods.
16 MiB RAM, stack at 0x80000000 (conventional RISC-V layout).

**Syscalls**: Minimal Linux ABI set for basic I/O.

**Debugging**: Register dumps on errors.

### RV32I Focus

Both tools implement the complete RV32I base (32 instructions).

---

## Features

### Assembler

- All RV32I instructions (R, I, S, B, U, J formats)
- Pseudoinstructions: `li`, `la`, `mv`, `nop`
- Full GAS-compatible `.section` directive support with arbitrary section names
- Data directives: `.ascii`, `.asciiz`, `.byte`, `.half`, `.word`, `.space`
- Forward/backward label references
- Accepts `.s` and `.asm` files
- **Debug mode**: Optional `--debug` flag for detailed assembly trace output

### Emulator

- Full RV32I execution
- Linux ABI syscalls: exit, read, write, openat, close, fstat, brk
- 16 MiB RAM, configurable load address
- Register dumps on errors
- Standard file I/O
- **Debug mode**: Optional `--debug` flag for fetch/decode/execute tracing

---

## Quick Reference

### Supported Instructions (RV32I)

| Type | Instructions |
|------|-------------|
| **Arithmetic** | `add`, `sub`, `addi` |
| **Logical** | `and`, `or`, `xor`, `andi`, `ori`, `xori` |
| **Shifts** | `sll`, `srl`, `sra`, `slli`, `srli`, `srai` |
| **Comparison** | `slt`, `sltu`, `slti`, `sltiu` |
| **Branches** | `beq`, `bne`, `blt`, `bge`, `bltu`, `bgeu` |
| **Jumps** | `jal`, `jalr` |
| **Loads** | `lb`, `lh`, `lw`, `lbu`, `lhu` |
| **Stores** | `sb`, `sh`, `sw` |
| **Upper Immediate** | `lui`, `auipc` |
| **System** | `ecall` |
| **Pseudo** | `li`, `la`, `mv`, `nop` |

### System Calls

| Syscall | Number | Usage |
|---------|--------|-------|
| `exit` | 93 | Terminate program with status code |
| `read` | 63 | Read from file descriptor |
| `write` | 64 | Write to file descriptor |
| `openat` | 56 | Open file (relative to directory fd) |
| `close` | 57 | Close file descriptor |
| `fstat` | 80 | Query file status |
| `brk` | 214 | Adjust program break (returns -ENOMEM) |

Unimplemented syscalls return -ENOSYS.

### Assembler Directives

| Directive | Description | Example |
|-----------|-------------|---------|
| `.text` | Switch to code section | `.text` |
| `.data` | Switch to data section | `.data` |
| `.rodata` | Switch to read-only data section | `.rodata` |
| `.bss` | Switch to uninitialized data section | `.bss` |
| `.section <name>` | Switch to arbitrary section (GAS-compatible) | `.section .text.startup` |
| `.ascii` | Store ASCII string | `.ascii "hello"` |
| `.asciiz` | Store null-terminated string | `.asciiz "world"` |
| `.byte` | Store 8-bit value(s) | `.byte 42, 0x2A` |
| `.half` | Store 16-bit value(s) | `.half 1000` |
| `.word` | Store 32-bit value(s) | `.word 0x12345678` |
| `.space` | Reserve N bytes | `.space 100` |

#### Section Directive Support

The assembler now supports full GNU Assembler (GAS) compatible `.section` directives, allowing you to specify arbitrary section names. This is essential for compatibility with standard RISC-V toolchains and linker scripts.

**Standard sections:**
- `.text` - Executable code section
- `.data` - Initialized data section
- `.rodata` - Read-only data section (constants, strings)
- `.bss` - Uninitialized data section (zero-initialized)

**Custom subsections:**
You can create custom subsections with any name, following GAS conventions:
- `.text.startup` - Startup/initialization code
- `.text.hot` - Hot/frequently executed code
- `.data.local` - Local data
- Any custom name like `.section .my_section`

**Example:**
```assembly
.section .text
main:
    addi x1, x0, 1

.section .rodata
constant:
    .word 42

.section .text.startup
_init:
    addi x2, x0, 2
```

The assembler automatically determines section types based on their names (e.g., `.text.*` sections are treated as code sections, `.data.*` as data sections).

---


## Getting Started

### Prerequisites

- G++ with C++17 support (GCC 7+)
- GNU Make
- Linux (for syscalls)

### Build

```bash
make
```

### Usage

Assemble:
```bash
./assembler/riscv_assembler program.s program.bin
```

Assemble with debug output:
```bash
./assembler/riscv_assembler --debug program.s program.bin
```

Run:
```bash
./emulator/riscv_emulator program.bin [load_address]
```

Run with debug tracing:
```bash
./emulator/riscv_emulator --debug program.bin
```

Test:
```bash
./unit_tests.sh
```

---

## Project Structure

## Project Structure

```
riscv-student-pack/
├── assembler/              # Object-oriented two-pass assembler
│   ├── include/
│   │   └── assembler.hpp  # Assembler class, symbol table, data structures
│   ├── src/
│   │   ├── main.cpp       # Entry point and CLI
│   │   ├── constructor.cpp # Assembler initialization
│   │   ├── first_pass.cpp # Symbol table construction
│   │   ├── second_pass.cpp # Instruction encoding
│   │   ├── encode.cpp     # Instruction format encoding
│   │   ├── expand_pseudoinstruction.cpp # Pseudoinstruction expansion
│   │   ├── adjust_labels.cpp # Label address adjustment
│   │   └── utils.cpp      # Parsing utilities
│   └── tests/
│       └── test_assembler.cpp # Unit tests with C++ assertions
│
├── emulator/              # Object-oriented RV32I instruction simulator
│   ├── include/
│   │   ├── cpu.hpp        # CPU class (registers as std::array, execution)
│   │   ├── memory.hpp     # Memory class (bounds-checked access)
│   │   └── instructions.hpp # Instruction class (decode/execute)
│   ├── src/
│   │   ├── main.cpp       # Entry point and CLI
│   │   ├── cpu.cpp        # Fetch-decode-execute, register management
│   │   ├── instructions.cpp # Instruction decoder and immediate extraction
│   │   └── memory.cpp     # Memory operations with alignment checks
│   └── tests/
│       └── test_emulator.cpp # Comprehensive unit tests (12 test suites)
│
├── Makefile               # Root build configuration
├── unit_tests.sh          # Automated test runner
├── LICENSE                # MIT License
└── README.md
```

---

## Implementation Highlights

### Modern C++ Features in Action

**CPU Registers**: `std::array<uint32_t, 32>` provides:
- Fixed-size container (known at compile time)
- Bounds checking with `.at()` method
- Zero-cost abstraction over raw arrays
- Iterator support for range-based loops

**Memory Management**: `std::vector<uint8_t>` for RAM:
- Automatic allocation/deallocation
- Dynamic sizing based on requirements
- Contiguous storage for cache efficiency
- No manual memory management

**Symbol Table**: `std::map<std::string, uint32_t>`:
- Automatic ordering by label name
- Logarithmic lookup time
- Type-safe key-value pairs

**Boolean Returns**: Methods return `bool` instead of error codes:
- `decode()` returns true/false for success
- `is_running()` checks CPU state
- More idiomatic C++ style

**Object Lifecycle**:
- Constructors initialize all state
- Destructors automatically clean up
- Copy/move semantics properly handled
- No dangling pointers or resource leaks

### Testing

Comprehensive unit test suites using C++ assertions:

**Emulator Tests** (12 test suites):
1. CPU initialization
2. Memory operations (8/16/32-bit, alignment)
3. Sign extension
4. Instruction decoding
5. Register operations
6. ALU operations
7. Load/store operations
8. Branch operations
9. System call operations
10. Complete CPU step execution
11. Complex instruction execution
12. Memory layout validation

**Assembler Tests**: Label resolution, instruction encoding, data directives.

Run all tests: `./unit_tests.sh`

---

## Debug Mode

Both the assembler and emulator support a `--debug` flag for detailed execution tracing, useful for learning and troubleshooting.

### Assembler Debug Output

With `--debug`, the assembler displays:
- Each line processed during assembly
- Pseudoinstruction expansions
- Detailed operand parsing breakdown
- Section switches and label definitions

**Example:**
```bash
./assembler/riscv_assembler --debug program.s program.bin
```

**Sample output:**
```
Processed line: 'li x10, 42'
Expanding pseudoinstruction: li -> addi x10, x0, 42
DEBUG PARSING:
  Function: encode
  Instruction Type: I-type
  rd=10, rs1=0, imm=42
```

### Emulator Debug Output

With `--debug`, the emulator shows:
- **Fetch**: Current PC and raw instruction (hex)
- **Decode**: Instruction name and format
- **Execute**: Decoded operands and execution details
- Register writes and memory operations
- Branch/jump targets

**Example:**
```bash
./emulator/riscv_emulator --debug program.bin
```

**Sample output:**
```
--- Instruction Cycle ---
Fetch: PC=0x00000000 Instruction=0x02a00513
Decode: addi (I-type)
Execute: rd=10 rs1=0 imm=42
Executed: addi x10, x0, 42
```

**Use cases:**
- Understanding instruction encoding
- Debugging assembly programs
- Learning RISC-V execution model
- Verifying pseudoinstruction expansion
- Tracing control flow

---

## Future Work

### Near-Term

**RV32M Extension**: Multiplication and division instructions.
Completes the baseline embedded profile. Could be implemented as additional instruction classes.

**More Syscalls**: Add `lseek`, directory ops, `mmap`.
Enables more realistic programs. Extend syscall handling in CPU class.

**Better Errors**: Line numbers in assembler with source tracking.
Instruction tracing in emulator with debug mode.

### Medium-Term

**ELF Format**: Generate standard object files.
Interop with GNU toolchain and GDB. Consider using ELFIO library.

**Multi-File Assembly**: Separate compilation and linking.
Mirrors real development workflows. Would require linker implementation.

**Disassembler**: Binary to assembly converter.
Aids debugging, demonstrates reversibility. Reverse the Instruction class logic.

### Long-Term

**Extensions**: Floating-point (F, D), atomics (A), compressed (C).
Could be implemented as derived instruction classes.

**Performance Tools**: Cycle counting, cache simulation.
Microarchitecture teaching. Add instrumentation to CPU class.

**Cross-Platform**: Abstract syscalls for Windows/macOS.
Strategy pattern for syscall implementation.

**Template-Based Instruction Dispatch**: Compile-time optimization using C++ templates.
Zero-overhead abstractions for instruction execution.

These are natural evolution paths, not requirements.
Suitable as student projects or incremental improvements.
The object-oriented design makes extension straightforward.

---

## Contributing

Contributions welcome! Especially:

- Test cases and example programs
- Documentation and tutorials
- Bug reports with reproducible cases
- Performance improvements that maintain clarity

Open an issue before major changes.

---

## License

MIT License. See [LICENSE](./LICENSE).

Free to use, modify, and extend for any purpose.
