# RISC-V Student Pack - Quick Start

Get up and running with the RISC-V assembler and emulator in under 5 minutes.

## Prerequisites

- G++ with C++17 support
- GNU Make
- Linux (for syscall support)

## Build

```bash
git clone <repository-url>
cd riscv-student-pack
make
```

## Your First Program

Create `hello.s`:

```assembly
.text
.globl _start

_start:
    # Write "Hello, RISC-V!\n" to stdout
    li a0, 1                    # stdout
    la a1, message              # buffer address
    li a2, 15                   # length
    li a7, 64                   # sys_write
    ecall

    # Exit
    li a0, 0                    # exit code
    li a7, 93                   # sys_exit
    ecall

.data
message:
    .ascii "Hello, RISC-V!\n"
```

## Assemble and Run

```bash
# Assemble
./assembler/riscv_assembler hello.s hello.bin

# Run
./emulator/riscv_emulator hello.bin
```

Output: `Hello, RISC-V!`

## Common Commands

### Build
```bash
make                    # Build both tools
make assembler          # Build assembler only
make emulator           # Build emulator only
```

### Clean
```bash
make clean              # Remove object files (fast rebuild)
make clean-all          # Remove everything including executables
```

### Test
```bash
make test               # Run all unit tests
./unit_tests.sh         # Alternative test runner
```

### Debug Mode
```bash
# See detailed assembly process
./assembler/riscv_assembler --debug program.s program.bin

# Trace instruction execution
./emulator/riscv_emulator --debug program.bin
```

## Quick Reference

### Supported Instructions

**Arithmetic**: `add`, `sub`, `addi`
**Logical**: `and`, `or`, `xor`, `andi`, `ori`, `xori`
**Shifts**: `sll`, `srl`, `sra`, `slli`, `srli`, `srai`
**Compare**: `slt`, `sltu`, `slti`, `sltiu`
**Branches**: `beq`, `bne`, `blt`, `bge`, `bltu`, `bgeu`
**Jumps**: `jal`, `jalr`
**Memory**: `lb`, `lh`, `lw`, `lbu`, `lhu`, `sb`, `sh`, `sw`
**Upper**: `lui`, `auipc`
**Pseudo**: `li`, `la`, `mv`, `nop`, `call`, `ret`, `j`

**Common pseudoinstructions:**
- `li rd, imm` - Load immediate (expands to `addi` or `lui`+`addi`)
- `la rd, label` - Load address (expands to `auipc`+`addi`)
- `mv rd, rs` - Copy register (expands to `addi rd, rs, 0`)
- `call label` - Call function (expands to `jal ra, label`)
- `ret` - Return from function (expands to `jalr x0, ra, 0`)
- `j label` - Unconditional jump (expands to `jal x0, label`)
- `nop` - No operation (expands to `addi x0, x0, 0`)
.byte 42                # 8-bit value
.half 1000              # 16-bit value
.word 0x12345678        # 32-bit value
.ascii "text"           # String (no null terminator)
.asciiz "text"          # Null-terminated string
.space 100              # Reserve 100 bytes
```

### System Calls

| Number | Name | Args | Description |
|--------|------|------|-------------|
| 93 | exit | a0=status | Terminate program |
| 63 | read | a0=fd, a1=buf, a2=len | Read from file |
| 64 | write | a0=fd, a1=buf, a2=len | Write to file |

Put syscall number in `a7`, arguments in `a0-a2`, then execute `ecall`.

## Example Programs

### Fibonacci

```assembly
.text
_start:
    li a0, 10           # Calculate fib(10)
    call fibonacci

    # Exit with result
    li a7, 93
    ecall

fibonacci:
    li t0, 0            # fib(0) = 0
    li t1, 1            # fib(1) = 1
    li t2, 0            # counter

fib_loop:
    bge t2, a0, fib_done
    add t3, t0, t1      # next = a + b
    mv t0, t1           # a = b
    mv t1, t3           # b = next
    addi t2, t2, 1      # counter++
    j fib_loop

fib_done:
    mv a0, t0
    ret
```

### Array Sum

```assembly
.text
_start:
    la a0, array        # array address
    li a1, 5            # array length
    call sum_array

    # Exit with sum in a0
    li a7, 93
    ecall

sum_array:
    li t0, 0            # sum = 0
    li t1, 0            # i = 0

sum_loop:
    bge t1, a1, sum_done
    slli t2, t1, 2      # offset = i * 4
    add t2, a0, t2      # address = base + offset
    lw t3, 0(t2)        # load array[i]
    add t0, t0, t3      # sum += array[i]
    addi t1, t1, 1      # i++
    j sum_loop

sum_done:
    mv a0, t0
    ret

.data
array:
    .word 10, 20, 30, 40, 50
```

## Troubleshooting

**Build fails**: Ensure G++ supports C++17 (GCC 7+)
```bash
g++ --version
```

**Segmentation fault**: Check memory addresses and alignment
```bash
./emulator/riscv_emulator --debug program.bin  # See execution trace
```

**Label errors**: Use `--debug` to see symbol resolution
```bash
./assembler/riscv_assembler --debug program.s program.bin
```

**Tests fail**: Clean and rebuild
```bash
make clean-all
make test
```

## Next Steps

- See [README.md](README.md) for architecture and design details
- Check `assembler/tests/` and `emulator/tests/` for more examples
- Run `make help` to see all available targets
- Explore section support with `.section` directive

## Getting Help

```bash
make help                                    # Show all make targets
./assembler/riscv_assembler                  # Show assembler usage
./emulator/riscv_emulator                    # Show emulator usage
```

For detailed documentation on design choices, OOP architecture, and implementation details, see [README.md](README.md).
