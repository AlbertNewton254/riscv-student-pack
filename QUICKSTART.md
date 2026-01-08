# RISC-V Student Pack - Quick Start

Action-oriented guide to get working code in 5 minutes.

## Prerequisites

Verify you have the required tools:

```bash
g++ --version        # Need 7.0 or higher
make --version       # Any recent version
uname -s             # Must be Linux
```

Stop and install missing tools before continuing.

## Build and Install

Clone and build the project:

```bash
git clone git@github.com:AlbertNewton254/riscv-student-pack.git
cd riscv-student-pack
make
```

You now have:
- assembler/riscv_assembler - Translates .s files to binaries
- emulator/riscv_emulator - Executes binary files

## Write a Program

Create hello.s:

```assembly
.text
.globl _start

_start:
    # Write "Hello, RISC-V!\n" to stdout
    li a0, 1                    # stdout (fd=1)
    la a1, message              # buffer address
    li a2, 15                   # number of bytes
    li a7, 64                   # syscall: write
    ecall

    # Exit program
    li a0, 0                    # exit code
    li a7, 93                   # syscall: exit
    ecall

.data
message:
    .asciiz "Hello, RISC-V!\n"
```

## Assemble

```bash
./assembler/riscv_assembler hello.s hello.bin
```

Success: hello.bin now contains executable code.

## Run

```bash
./emulator/riscv_emulator hello.bin
```

Expected output: `Hello, RISC-V!`

## More Examples

### Simple Arithmetic

File: add.s

```assembly
.text
_start:
    li x1, 42
    li x2, 8
    add x3, x1, x2
    # x3 = 50

    li a0, 0
    li a7, 93
    ecall
```

### Loop (Count to 10)

File: loop.s

```assembly
.text
_start:
    li x1, 0        # counter

loop:
    addi x1, x1, 1  # x1++
    li x2, 10
    bne x1, x2, loop    # if x1 != 10, go back

    li a0, 0
    li a7, 93
    ecall
```

### Using Memory

File: memory.s

```assembly
.text
_start:
    la x1, data     # Load address of data
    lw x2, 0(x1)    # Load first word
    lw x3, 4(x1)    # Load second word
    add x4, x2, x3  # Add them

    li a0, 0
    li a7, 93
    ecall

.data
data:
    .word 100
    .word 200
```

## Common Commands

| Task | Command |
|------|---------|
| Build | `make` |
| Assemble | `./assembler/riscv_assembler prog.s prog.bin` |
| Run | `./emulator/riscv_emulator prog.bin` |
| Test | `make test` |
| Debug assembler | `./assembler/riscv_assembler --debug prog.s prog.bin` |
| Debug emulator | `./emulator/riscv_emulator --debug prog.bin` |
| Clean | `make clean` |
| Help | `make help` |

## Assemble and Run in One Command

```bash
./assembler/riscv_assembler program.s program.bin && \
./emulator/riscv_emulator program.bin
```

## Debug with Trace Output

### Assembler Debug

```bash
./assembler/riscv_assembler --debug program.s program.bin
```

Shows each line processed, label resolution, and pseudoinstruction expansion.

### Emulator Debug

```bash
./emulator/riscv_emulator --debug program.bin
```

Shows fetch/decode/execute for every instruction.

## Instruction Cheat Sheet

### Load a Value

```assembly
li x1, 42           # Load immediate
li x1, 0x12345678   # For larger values, expands to lui+addi
```

### Load an Address

```assembly
la x1, label        # Load address of label
la x1, data         # Works for data section too
```

### Arithmetic

```assembly
add x1, x2, x3      # x1 = x2 + x3
addi x1, x2, 10     # x1 = x2 + 10
sub x1, x2, x3      # x1 = x2 - x3
```

### Branches

```assembly
beq x1, x2, label   # If x1 == x2, jump to label
bne x1, x2, label   # If x1 != x2, jump to label
blt x1, x2, label   # If x1 < x2, jump (signed)
bge x1, x2, label   # If x1 >= x2, jump (signed)
```

### Jumps

```assembly
j label             # Unconditional jump to label
jal ra, label       # Jump and link (call subroutine)
ret                 # Return from subroutine
```

### Memory

```assembly
lw x1, 0(x2)        # Load word from address in x2
sw x1, 0(x2)        # Store word to address in x2
lh x1, 0(x2)        # Load halfword (16-bit)
sh x1, 0(x2)        # Store halfword
lb x1, 0(x2)        # Load byte
sb x1, 0(x2)        # Store byte
```

### Logical

```assembly
and x1, x2, x3      # x1 = x2 & x3
or x1, x2, x3       # x1 = x2 | x3
xor x1, x2, x3      # x1 = x2 ^ x3
```

### Shifts

```assembly
sll x1, x2, x3      # x1 = x2 << x3
srl x1, x2, x3      # x1 = x2 >> x3 (logical)
sra x1, x2, x3      # x1 = x2 >> x3 (arithmetic)
```

### System Calls

```assembly
# Write to file descriptor
li a0, 1            # fd = stdout (1)
la a1, message      # buffer address
li a2, 5            # length
li a7, 64           # syscall: write
ecall

# Exit
li a0, 0            # exit code
li a7, 93           # syscall: exit
ecall
```

## Registers (ABI Names)

| Name | Use | Preserved? |
|------|-----|-----------|
| x0 | zero (always 0) | - |
| x1/ra | return address | Caller |
| x2/sp | stack pointer | - |
| x5/t0-x7/t2 | temporaries | Caller |
| x10/a0-x11/a1 | function args/return | Caller |
| x12-17 | more args | Caller |
| x27-31 | saved (s0-s4) | Callee |

For simple programs, use any registers. This matters for function calls.

## Data Directives

| Directive | Example | Result |
|-----------|---------|--------|
| .word | .word 42 | 4 bytes: 42 |
| .word | .word 1, 2, 3 | 3 x 4 bytes |
| .half | .half 1000 | 2 bytes: 1000 |
| .byte | .byte 65 | 1 byte: 65 ('A') |
| .ascii | .ascii "hi" | 2 bytes: "hi" (no null) |
| .asciiz | .asciiz "hi" | 3 bytes: "hi\0" |
| .space | .space 100 | 100 zero bytes |

## Sections

```assembly
.text               # Code section (default)
_start:
    # instructions here

.data               # Initialized data
values:
    .word 1, 2, 3

.rodata             # Read-only data (strings, constants)
message:
    .asciiz "hello"

.bss                # Uninitialized data (not supported in emulator)
buffer:
    .space 100
```

## Project Structure

```
riscv-student-pack/
|-- assembler/
|   |-- riscv_assembler     Use this
|   '-- [source files]
|-- emulator/
|   |-- riscv_emulator      Use this
|   '-- [source files]
|-- tests/                  Explore examples here
|-- Makefile
|-- README.md               Full documentation
'-- QUICKSTART.md           This file
```

## Getting Help

- Full documentation: See README.md
- Assembler details: See assembler/README.md
- Emulator details: See emulator/README.md
- Test examples: See tests/README.md
- Build targets: Run `make help`
- Trace assembly: Run `./assembler/riscv_assembler --debug`
- Trace execution: Run `./emulator/riscv_emulator --debug`

## What Works

- All 32 RV32I base instructions
- Pseudoinstructions (li, la, mv, call, ret, j, nop)
- Forward and backward labels
- Function calls with call/ret
- Memory access (load/store)
- System calls (exit, write, read)
- Debug mode for both tools

## What Does Not Work

- Floating point (requires RV32F)
- Atomics (requires RV32A)
- Multiplication/division (requires RV32M)
- Compressed instructions (requires RVC)

RV32I is the baseline. Everything above requires extensions.

## Next Steps

You now know enough to write real RISC-V programs. Go build something!