# RISC-V Student Pack

Educational toolkit for RISC-V Assembly: assembler and emulator in C (currently RV32I only).

## Overview


RISC-V Student Pack provides students and educators with a simple, extensible RISC-V environment:

- **Assembler**: Two-pass assembler for RV32I Assembly files (in development).
- **Emulator**: RV32I emulator supporting all base instructions and a limited set of Linux ABI syscalls.

Both tools are written in C and designed for easy compilation and use on Linux systems.

**Note:** The emulator currently implements only the RV32I base instruction set. Support for the M extension (multiplication/division) and additional syscalls is planned for future releases.

---

## Table of Contents
1. [Features](#features)
2. [Directory Structure](#directory-structure)
3. [Prerequisites](#prerequisites)
4. [Building](#building)
5. [Usage](#usage)
6. [License](#license)
7. [Contributing](#contributing)

---

## Features

### Assembler
- Two-pass parsing: collects labels/symbols, then encodes instructions and pseudoinstructions.
- Input: `.asm` or `.s` files.
- Output: `.o` object files.
- **Planned**: The assembler has not been implemented yet.

### Emulator
- Supports all RV32I instructions: arithmetic, logic, comparisons, branches, loads/stores, jumps.
- Emulates a limited set of Linux ABI syscalls (e.g., exit, read, write, openat, close, brk, fstat).
- **Planned:** Support for RV32IM (multiplication/division) and more syscalls.

---

## Directory Structure

```
riscv-student-pack/
├── README.md
├── assembler/
│   └── ...
├── emulator/
│   ├── cpu.c
│   ├── cpu.h
│   ├── instructions.c
│   ├── instructions.h
│   ├── main.c
│   ├── memory.c
│   └── memory.h
└── LICENSE
```

---

## Prerequisites

- GCC (or compatible C compiler)
- Linux (for handling syscalls)

---

## Building

### Assembler

```bash
cd assembler
gcc -Wall -Wextra -Wpedantic -std=c99 -Iinclude src/*.c -O2 -o riscv_assembler
```

### Emulator

```bash
cd emulator
gcc -Wall -Wextra -Wpedantic -std=c99 -Iinclude src/*.c -O2 -o riscv_emulator
```

---

## Usage

### Assembler

```bash
./riscv_assembler <filename.asm|filename.s>
```
Output: `<filename.o>`

### Emulator

```bash
./riscv_emulator <program.bin> [load_address]
```
Output: Runs the binary and prints exit status or error information.

---

## License

This project is released under the [MIT License](./LICENSE). For educational use only.

---

## Roadmap

- [ ] Add support for RV32IM (M extension: multiplication/division)
- [ ] Expand the set of supported Linux syscalls
- [ ] Improve error reporting and debugging features

---

## Contributing & Contact

Contributions, suggestions, and bug reports are welcome! Please open an issue or pull request.