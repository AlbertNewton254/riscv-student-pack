# RISC-V Student Pack

Educational toolkit for RISC-V Assembly: assembler and emulator in C (currently RV32I only).

## Overview

RISC-V Student Pack provides students and educators with a simple, extensible RISC-V environment:

- **Assembler**: Two-pass assembler for RV32I Assembly files.
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
- Two-pass assembler: first pass resolves labels and calculates addresses, second pass encodes instructions and outputs binary.
- Supports a subset of RV32I instructions (see below).
- Handles `.text` and `.data` sections, with label relocation for data.
- Supports basic assembler directives:
	- `.text`, `.data` (section switching)
	- `.ascii` (inline ASCII string data)
	- `.byte` (single byte data)
- Input: `.s`/`.asm` files (plain text assembly)
- Output: flat binary file (not ELF)
- **Planned:** Pseudoinstructions, full RV32IM, more directives, and more syscalls.

#### Supported Assembly Instructions

- `add rd, rs1, rs2` (R-type)
- `addi rd, rs1, imm` (I-type)
- `lui rd, imm` (U-type)
- `ecall` (system call)

> **Note:** Only these instructions are currently recognized by the assembler. Other instructions will result in an error.

#### Supported Assembler Directives

- `.text` — Switch to code section
- `.data` — Switch to data section
- `.ascii "string"` — Store ASCII string in data section
- `.byte N` — Store single byte value in data section

### Emulator
- Supports all RV32I instruction formats (R, I, S, B, U, J) for decoding and execution.
- Implements instruction types: arithmetic, logic, comparisons, branches, loads/stores, jumps, system calls.
- Emulates a limited set of Linux ABI syscalls (see below).
- Memory-mapped, byte-addressable RAM (16 MiB by default).
- Stack pointer initialized to 0x80000000.
- Prints register dump on error or exit.
- **Planned:** RV32IM (multiplication/division), more syscalls, improved debugging.

#### Supported Syscalls (RV32 Linux ABI)

| Syscall Name | Number | Description |
|-------------|--------|-------------|
| exit        | 93     | Exit program |
| read        | 63     | Read from file descriptor |
| write       | 64     | Write to file descriptor |
| openat      | 56     | Open file |
| close       | 57     | Close file descriptor |
| brk         | 214    | Set program break (stub, returns -ENOMEM) |
| fstat       | 80     | Get file status |

Other syscalls will return -ENOSYS (not implemented).

#### Supported Instruction Formats (Emulator)

- R-type: arithmetic/logical (e.g., add, sub, and, or, xor, slt, sll, sra, srl)
- I-type: immediate arithmetic/logical, loads, jalr, ecall
- S-type: stores
- B-type: branches (beq, bne, blt, bge, bltu, bgeu)
- U-type: lui, auipc
- J-type: jal

> **Note:** The assembler only emits a subset of these; the emulator can execute all RV32I instructions if present in the binary.

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
- Linux (for handling syscalls, since they rely on unistd.h header)

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

### Emulator

- [ ] Add support for RV32IM (M extension: multiplication/division)
- [ ] Expand the set of supported Linux syscalls
- [ ] Improve error reporting and debugging features

### Assembler

- [ ] Add support to pseudoinstructions

---

## Contributing & Contact

Contributions, suggestions, and bug reports are welcome! Please open an issue or pull request.
