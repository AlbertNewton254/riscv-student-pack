# RISC-V Student Pack

Educational toolkit for learning RISC-V: assembler and emulator in modern C++ (previously in C).

## Overview

Two complementary tools for RISC-V assembly programming:

**Assembler**: Two-pass assembler translating RV32I assembly to binaries.
First pass resolves labels, second pass encodes instructions.

**Emulator**: Instruction simulator executing RV32I binaries.
Implements base integer instructions with Linux ABI syscalls.

Both written in modern C++ (C++17) for clarity, safety, and extensibility.

---

## Design Choices

### Modern C++ Features

**Smart Pointers**: RAII-based file management with `std::unique_ptr`.
Automatic resource cleanup, no manual fclose() needed.

**Dynamic Containers**: `std::vector` for label storage.
No arbitrary limits, grows as needed.

**Type Safety**: C++ standard library headers and stricter type checking.
Catch errors at compile time.

### Assembler

Two-pass architecture for simple label resolution:
- Pass 1: Build symbol table, calculate addresses
- Pass 2: Encode instructions with resolved symbols

**Binary Output**: Flat binaries (not ELF) for simplicity.
What you see is what executes.

**Pseudoinstructions**: `li`, `la`, `mv`, `nop` expand to RV32I.
Idiomatic assembly, pure execution.

**Sections**: `.text` and `.data` with automatic address calculation.

### Emulator

Fetch-decode-execute cycle with 16 MiB RAM.

**Memory**: Stack at 0x80000000, conventional RISC-V layout.

**Syscalls**: Minimal Linux ABI set for basic I/O.

**Debugging**: Register dumps on errors.

### RV32I Focus

Both tools implement the complete RV32I base (32 instructions).

---

## Features

### Assembler

- All RV32I instructions (R, I, S, B, U, J formats)
- Pseudoinstructions: `li`, `la`, `mv`, `nop`
- `.text` and `.data` sections
- Data directives: `.ascii`, `.asciiz`, `.byte`, `.half`, `.word`, `.space`
- Forward/backward label references
- Accepts `.s` and `.asm` files

### Emulator

- Full RV32I execution
- Linux ABI syscalls: exit, read, write, openat, close, fstat, brk
- 16 MiB RAM, configurable load address
- Register dumps on errors
- Standard file I/O

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
| `.ascii` | Store ASCII string | `.ascii "hello"` |
| `.asciiz` | Store null-terminated string | `.asciiz "world"` |
| `.byte` | Store 8-bit value(s) | `.byte 42, 0x2A` |
| `.half` | Store 16-bit value(s) | `.half 1000` |
| `.word` | Store 32-bit value(s) | `.word 0x12345678` |
| `.space` | Reserve N bytes | `.space 100` |

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

Run:
```bash
./emulator/riscv_emulator program.bin [load_address]
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
├── assembler/              # Two-pass assembler implementation
│   ├── include/
│   │   └── assembler.hpp  # Public API and data structures
│   ├── src/
│   │   ├── main.cpp       # Entry point and CLI
│   │   ├── first_pass.cpp # Symbol table construction
│   │   ├── second_pass.cpp # Instruction encoding
│   │   ├── encode.cpp     # Instruction format encoding
│   │   ├── expand_pseudoinstruction.cpp
│   │   ├── adjust_labels.cpp
│   │   └── utils.cpp      # Parsing utilities
│   └── tests/
│       └── test_assembler.cpp
│
├── emulator/              # RV32I instruction simulator
│   ├── include/
│   │   ├── cpu.hpp        # CPU state and control
│   │   ├── memory.hpp     # Memory subsystem
│   │   └── instructions.hpp # Instruction decoder
│   ├── src/
│   │   ├── main.cpp       # Entry point and CLI
│   │   ├── cpu.cpp        # Fetch-decode-execute loop
│   │   ├── instructions.cpp # Instruction implementations
│   │   └── memory.cpp     # Memory operations
│   └── tests/
│       └── test_emulator.cpp
│
├── Makefile               # Root build configuration
├── unit_tests.sh          # Test runner
└── README.md
```

---

## Future Work

### Near-Term

**RV32M Extension**: Multiplication and division instructions.
Completes the baseline embedded profile.

**More Syscalls**: Add `lseek`, directory ops, `mmap`.
Enables more realistic programs.

**Better Errors**: Line numbers in assembler.
Instruction tracing in emulator.

### Medium-Term

**ELF Format**: Generate standard object files.
Interop with GNU toolchain and GDB.

**Multi-File Assembly**: Separate compilation and linking.
Mirrors real development workflows.

**Disassembler**: Binary to assembly converter.
Aids debugging, demonstrates reversibility.

### Long-Term

**Extensions**: Floating-point (F, D), atomics (A), compressed (C).

**Performance Tools**: Cycle counting, cache simulation.
Microarchitecture teaching.

**Cross-Platform**: Abstract syscalls for Windows/macOS.

These are natural evolution paths, not requirements.
Suitable as student projects or incremental improvements.

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
