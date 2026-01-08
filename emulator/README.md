# RISC-V Emulator

Instruction simulator executing RV32I binaries with full fetch-decode-execute pipeline, memory simulation, and Linux ABI syscalls.

## Brief Description

The emulator executes RISC-V machine code binaries in a simulated environment. It implements a complete fetch-decode-execute pipeline with 32 registers, configurable memory (default 16 MiB), and support for Linux ABI syscalls. The emulator can trace execution with debug mode for learning and debugging RISC-V programs.

## Folder Structure

```
emulator/
├── Makefile                 Build configuration
├── README.md                This file
├── riscv_emulator           Executable
├── include/
│   ├── cpu.hpp              CPU class and registers
│   ├── memory.hpp           Memory management
│   └── instructions.hpp     Instruction decoding
└── src/
    ├── main.cpp             Entry point and CLI
    ├── cpu.cpp              CPU fetch-decode-execute
    ├── memory.cpp           Memory operations
    └── instructions.cpp     Instruction formatting
```

## Features

- Full RV32I fetch-decode-execute pipeline
- 32 registers with standard ABI names
- Configurable memory (default 16 MiB) with bounds checking
- Linux ABI syscalls: exit, read, write, openat, close, fstat, brk
- Register dumps and stack traces on errors
- Debug mode with instruction tracing
- Alignment validation and error detection

## Documentation

### Architecture


#### Fetch-Decode-Execute Pipeline

The emulator implements a classic processor pipeline:

1. Fetch: Load instruction from memory at program counter
2. Decode: Parse instruction format and extract operands
3. Execute: Perform operation and update processor state
4. Repeat until program exit or error

#### Core Components

**CPU Class** (include/cpu.hpp, src/cpu.cpp)
- 32 registers: std::array<uint32_t, 32>
- Program counter and execution state
- Fetch-decode-execute methods
- Register manipulation with bounds checking
- Exception handling and error reporting

**Memory Class** (include/memory.hpp, src/memory.cpp)
- RAM: std::vector<uint8_t> (16 MiB default)
- Bounds checking on all access
- Alignment validation
- Word/halfword/byte read/write
- Stack at 0x80000000 (conventional RISC-V)

**Instruction Class** (include/instructions.hpp, src/instructions.cpp)
- Instruction decoding
- Format detection (R, I, S, B, U, J)
- Immediate value extraction and sign extension
- Type-safe representation

### Memory Layout

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

### Instruction Set

#### All 32 RV32I Base Instructions

##### Arithmetic (3)

- add rd, rs1, rs2 - rd = rs1 + rs2
- sub rd, rs1, rs2 - rd = rs1 - rs2
- addi rd, rs1, imm - rd = rs1 + imm

##### Logical (6)

- and rd, rs1, rs2 - rd = rs1 & rs2
- or rd, rs1, rs2 - rd = rs1 | rs2
- xor rd, rs1, rs2 - rd = rs1 ^ rs2
- andi rd, rs1, imm - rd = rs1 & imm
- ori rd, rs1, imm - rd = rs1 | imm
- xori rd, rs1, imm - rd = rs1 ^ imm

##### Shifts (6)

- sll rd, rs1, rs2 - rd = rs1 << rs2[4:0]
- srl rd, rs1, rs2 - rd = rs1 >> rs2[4:0] (logical)
- sra rd, rs1, rs2 - rd = rs1 >> rs2[4:0] (arithmetic)
- slli rd, rs1, shamt - rd = rs1 << shamt
- srli rd, rs1, shamt - rd = rs1 >> shamt (logical)
- srai rd, rs1, shamt - rd = rs1 >> shamt (arithmetic)

##### Comparison (4)

- slt rd, rs1, rs2 - rd = (rs1 < rs2) ? 1 : 0 (signed)
- sltu rd, rs1, rs2 - rd = (rs1 < rs2) ? 1 : 0 (unsigned)
- slti rd, rs1, imm - rd = (rs1 < imm) ? 1 : 0 (signed)
- sltiu rd, rs1, imm - rd = (rs1 < imm) ? 1 : 0 (unsigned)

##### Branches (6)

- beq rs1, rs2, label - PC = (rs1 == rs2) ? label : PC+4
- bne rs1, rs2, label - PC = (rs1 != rs2) ? label : PC+4
- blt rs1, rs2, label - PC = (rs1 < rs2) ? label : PC+4 (signed)
- bge rs1, rs2, label - PC = (rs1 >= rs2) ? label : PC+4 (signed)
- bltu rs1, rs2, label - PC = (rs1 < rs2) ? label : PC+4 (unsigned)
- bgeu rs1, rs2, label - PC = (rs1 >= rs2) ? label : PC+4 (unsigned)

##### Jumps (2)

- jal rd, label - rd = PC+4; PC = label
- jalr rd, rs1, imm - rd = PC+4; PC = (rs1 + imm) & ~1

##### Loads (5)

- lw rd, offset(rs1) - rd = mem[rs1 + offset] (32-bit)
- lh rd, offset(rs1) - rd = mem[rs1 + offset] (16-bit, sign-extend)
- lhu rd, offset(rs1) - rd = mem[rs1 + offset] (16-bit, zero-extend)
- lb rd, offset(rs1) - rd = mem[rs1 + offset] (8-bit, sign-extend)
- lbu rd, offset(rs1) - rd = mem[rs1 + offset] (8-bit, zero-extend)

##### Stores (3)

- sw rs2, offset(rs1) - mem[rs1 + offset] = rs2 (32-bit)
- sh rs2, offset(rs1) - mem[rs1 + offset] = rs2 (16-bit)
- sb rs2, offset(rs1) - mem[rs1 + offset] = rs2 (8-bit)

##### Upper Immediate (2)

- lui rd, imm - rd = imm << 12
- auipc rd, imm - rd = PC + (imm << 12)

##### System (1)

- ecall - System call (a7 = syscall number, a0-a2 = args)

### Registers

| x0 | x1 | x2 | x3 | x4 | x5 | x6 | x7 | x8 | x9 |
|----|----|----|----|----|----|----|----|----|-----|
| zero | ra | sp | gp | tp | t0 | t1 | t2 | s0 | s1 |

Full 32-register set with standard ABI names.

### System Calls

Minimal Linux ABI syscalls for basic I/O and process control.

| Number | Name | Arguments | Returns | Purpose |
|--------|------|-----------|---------|---------|
| 93 | exit | a0=code | - | Exit with status |
| 63 | read | a0=fd, a1=buf, a2=len | count | Read from file |
| 64 | write | a0=fd, a1=buf, a2=len | count | Write to file |
| 56 | openat | a0=dirfd, a1=path, a2=flags | fd | Open file |
| 57 | close | a0=fd | 0 | Close file |
| 80 | fstat | a0=fd, a1=buf | 0 | Query file |
| 214 | brk | a0=addr | new_brk | Heap control |

Standard file descriptors:
- 0 = stdin
- 1 = stdout
- 2 = stderr

### Usage

#### Run Program

```bash
./riscv_emulator program.bin
```

#### Debug Mode

```bash
./riscv_emulator --debug program.bin
```

Shows for each instruction:
```
--- Instruction Cycle ---
Fetch: PC=0x00000000 Instruction=0x02a00513
Decode: addi (I-type)
Execute: rd=10 rs1=0 imm=42
Executed: addi x10, x0, 42
```

#### Options

```
--debug         Trace execution (fetch/decode/execute)
--memory SIZE   Set RAM size in bytes (default: 16MB)
--load-at ADDR  Load program at address (default: 0x00000000)
```

### Implementation Details

#### Source Files

- main.cpp - Entry point, argument parsing, binary loading
- cpu.cpp - CPU state, fetch-decode-execute, register ops
- instructions.cpp - Instruction decoding and formatting
- memory.cpp - Memory access with bounds checking

#### Instruction Encoding

RV32I uses 32-bit fixed-width instructions:

- R-format: opcode[6:0] rd[11:7] funct3[14:12] rs1[19:15] rs2[24:20] funct7[31:25]
- I-format: opcode[6:0] rd[11:7] funct3[14:12] rs1[19:15] imm[31:20]
- S-format: opcode[6:0] imm[11:7] funct3[14:12] rs1[19:15] rs2[24:20] imm[31:25]
- B-format: opcode[6:0] imm[11] rd[11:7] funct3[14:12] rs1[19:15] rs2[24:20] imm[31:25]
- U-format: opcode[6:0] rd[11:7] imm[31:12]
- J-format: opcode[6:0] rd[11:7] imm[31:12]

#### Error Handling

Detects and reports:
- Invalid instructions (unsupported opcodes)
- Memory access violations (out of bounds)
- Alignment errors (misaligned loads/stores)
- Segmentation faults (null pointer dereference)
- Stack overflow (stack at 0x80000000)

Register dump on error shows:
- All 32 registers with values
- Program counter
- Last executed instruction
- Memory around fault address

### Build

#### Targets

```bash
make              Build emulator executable
make run PROGRAM=test.bin    Build and run
make clean        Remove object files
make clean-all    Remove everything
make test         Run unit tests (via root Makefile)
make format       Format code
make analyze      Static analysis
make debug        Debug build
make release      Optimized build
```

#### Compilation

Same flags as assembler:
- Standard: -O2 -Wall -Wextra -Werror -std=c++17
- Debug: -O0 -g -DDEBUG
- Release: -O3 -DNDEBUG

### Testing

Unit tests cover:
- CPU initialization and registers
- Memory operations (word/half/byte, alignment)
- Instruction execution (all formats)
- Sign extension and immediate handling
- Branch and jump logic
- Load/store operations
- Syscall handling
- Complex multi-instruction sequences

Run: `make test` (from root) or `cd ../tests && make run_test_emulator`

### Examples

#### Simple Arithmetic

```assembly
.text
_start:
    li x1, 42
    li x2, 8
    add x3, x1, x2
    # x3 = 50
```

#### Loop with Output

```assembly
.text
_start:
    li x1, 1        # counter

loop:
    li a0, 1        # stdout
    mv a1, x1       # value to output (simplified)
    li a2, 1        # length
    li a7, 64       # write syscall
    ecall

    addi x1, x1, 1
    li x2, 11
    bne x1, x2, loop

    # Exit
    li a0, 0
    li a7, 93
    ecall
```

#### Memory Access

```assembly
.text
_start:
    la x1, data     # Load data address
    lw x2, 0(x1)    # Load first word
    lw x3, 4(x1)    # Load second word
    add x4, x2, x3  # Add them

.data
data:
    .word 100, 200
```

## See Also

- [Main README](../README.md) - Project overview
- [QUICKSTART](../QUICKSTART.md) - Usage guide
- [Assembler](../assembler/README.md) - Binary generation
- [Tests](../tests/README.md) - Test infrastructure