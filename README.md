# RISC-V Student Pack

Educational toolkit for RISC-V Assembly: assembler and emulator in C (RV32I base, with pseudoinstructions and full test suite).

## Overview


RISC-V Student Pack provides students and educators with a simple, extensible RISC-V environment:

- **Assembler**: Two-pass assembler for RV32I Assembly files, supporting all base instructions and common pseudoinstructions.
- **Emulator**: RV32I emulator supporting all base instructions and a set of Linux ABI syscalls.

Both tools are written in C and designed for easy compilation and use on Linux systems.

**Note:** The emulator and assembler currently implement the RV32I base instruction set and common pseudoinstructions. Support for the M extension (multiplication/division) and additional syscalls is planned for future releases.

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
- Supports **all RV32I base instructions** (see below).
- Implements common pseudoinstructions: `li`, `la`, `mv`, `nop`.
- Handles `.text` and `.data` sections, with label relocation for data.
- Supports assembler directives:
    - `.text`, `.data` (section switching)
    - `.ascii`, `.asciiz` (inline ASCII string data, with/without null terminator)
    - `.byte`, `.half`, `.word`, `.space` (data allocation)
- Input: `.s`/`.asm` files (plain text assembly)
- Output: flat binary file (not ELF)


#### Supported Assembly Instructions

- All RV32I base instructions (R, I, S, B, U, J types)
- Pseudoinstructions: `li`, `la`, `mv`, `nop`

> **Note:** The assembler will report an error for instructions outside the RV32I base set or unsupported pseudoinstructions.


#### Supported Assembler Directives

- `.text` — Switch to code section
- `.data` — Switch to data section
- `.ascii "string"` — Store ASCII string in data section
- `.asciiz "string"` — Store null-terminated ASCII string
- `.byte N,...` — Store single byte(s) in data section
- `.half N,...` — Store 2-byte halfword(s) in data section
- `.word N,...` — Store 4-byte word(s) in data section
- `.space N` — Reserve N bytes in data section


### Emulator
- Supports all RV32I instruction formats (R, I, S, B, U, J) for decoding and execution.
- Implements instruction types: arithmetic, logic, comparisons, branches, loads/stores, jumps, system calls.
- Emulates a set of Linux ABI syscalls (see below).
- Memory-mapped, byte-addressable RAM (16 MiB by default).
- Stack pointer initialized to 0x80000000.
- Prints register dump on error or exit.


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

> **Note:** The assembler emits all RV32I instructions and supported pseudoinstructions; the emulator can execute all RV32I instructions present in the binary.

---


## Directory Structure

```
riscv-student-pack/
├── README.md
├── LICENSE
├── Makefile
├── unit_tests.sh
├── assembler/
│   ├── Makefile
│   ├── include/
│   │   └── assembler.h
│   ├── src/
│   │   ├── adjust_labels.c
│   │   ├── encode.c
│   │   ├── expand_pseudoinstruction.c
│   │   ├── first_pass.c
│   │   ├── main.c
│   │   ├── second_pass.c
│   │   └── utils.c
│   └── tests/
│       └── test_assembler.c
├── emulator/
│   ├── Makefile
│   ├── riscv_emulator
│   ├── include/
│   │   ├── cpu.h
│   │   ├── instructions.h
│   │   └── memory.h
│   ├── src/
│   │   ├── cpu.c
│   │   ├── instructions.c
│   │   ├── main.c
│   │   └── memory.c
│   └── tests/
│       └── test_emulator.c
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
make
```

### Emulator

```bash
cd emulator
make
```

---


## Usage

### Assembler

```bash
./riscv_assembler <input.s> <output.bin>
```
Output: `<output.bin>`

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
- [x] Add support to pseudoinstructions
- [ ] Add support to RV32IM (M extension: multiplication/division)
- [ ] Generate ELF (Executable and Linkable File) for full UNIX/Linux compatibility
- [ ] Allow more than one source file for assembling

---


## Testing

The project includes unit and integration tests for both assembler and emulator. Run all tests with:

```bash
./unit_tests.sh
```

This script builds both tools, runs unit tests, and performs integration tests (including pseudoinstructions and memory operations).

## Contributing & Contact

Contributions, suggestions, and bug reports are welcome! Please open an issue or pull request.
