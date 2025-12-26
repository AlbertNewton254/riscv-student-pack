.text
_start:
    # a0 = 1 (stdout)
    addi x10, x0, 1

    addi x11, x0, msg

    # a2 = tamanho
    addi x12, x0, 15

    # a7 = write
    addi x17, x0, 64
    ecall

    # exit(0)
    addi x10, x0, 0
    addi x17, x0, 93
    ecall

.data
msg:
    .asciiz "Hello, RISC-V!\n"
