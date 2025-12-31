/* cpu.c */
#include "cpu.h"
#include "memory.h"
#include "instructions.h"
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>

cpu_t *cpu_init(void) {
	cpu_t *cpu = (cpu_t *)malloc(sizeof(cpu_t));
	if (!cpu) {
		return NULL;
	}

	for (int i = 0; i < 32; i++) {
		cpu->x[i] = 0;
	}
	cpu->pc = 0;
	cpu->running = 1;

	cpu->x[2] = STACK_TOP;

	return cpu;
}

void cpu_destroy(cpu_t *cpu) {
	if (cpu) {
		free(cpu);
	}
}

cpu_status_t cpu_fetch(cpu_t *cpu, memory_t *mem, uint32_t *instruction) {
	memory_status_t status = memory_read32(mem, cpu->pc, instruction);

	if (status == MEM_OK) {
		cpu->pc += 4;
		return CPU_OK;
	}
	else if (status == MEM_MISALIGNED_ERROR) {
		return CPU_FETCH_MISALIGNED;
	}
	else if (status == MEM_READ_ERROR) {
		return CPU_FETCH_OUT_OF_BOUNDS;
	}

	return CPU_FETCH_ERROR;
}

static uint32_t reg_read(cpu_t *cpu, uint8_t reg) {
	return (reg == 0) ? 0 : cpu->x[reg];
}

static void reg_write(cpu_t *cpu, uint8_t reg, uint32_t value) {
	if (reg != 0) {
		cpu->x[reg] = value;
	}
}

static cpu_status_t handle_syscall(cpu_t *cpu, memory_t *mem) {
	uint32_t syscall_num = cpu->x[17];
	uint32_t arg1 = cpu->x[10];
	uint32_t arg2 = cpu->x[11];
	uint32_t arg3 = cpu->x[12];

	switch (syscall_num) {
		case SYS_exit: {
			cpu->running = 0;
			return CPU_SYSCALL_EXIT;
		}

		case SYS_write: {
			int fd = (int)arg1;
			uint32_t buf_addr = arg2;
			size_t count = (size_t)arg3;

			if (buf_addr + count > mem->size) {
				cpu->x[10] = -1;
				break;
			}

			ssize_t result = write(fd, &mem->data[buf_addr], count);
			cpu->x[10] = (uint32_t)result;
			break;
		}

		case SYS_read: {
			int fd = (int)arg1;
			uint32_t buf_addr = arg2;
			size_t count = (size_t)arg3;

			if (buf_addr + count > mem->size) {
				cpu->x[10] = -1;
				break;
			}

			ssize_t result = read(fd, &mem->data[buf_addr], count);
			cpu->x[10] = (uint32_t)result;
			break;
		}

		case SYS_openat: {
			uint32_t path_addr = arg2;
			int flags = (int)arg3;
			mode_t mode = (mode_t)arg3;

			char path[256];
			int i;
			for (i = 0; i < (int)sizeof(path) - 1; i++) {
				if (path_addr + i >= mem->size) break;
				path[i] = mem->data[path_addr + i];
				if (path[i] == '\0') break;
			}
			path[i] = '\0';

			int result = open(path, flags, mode);
			cpu->x[10] = (uint32_t)result;
			break;
		}

		case SYS_close: {
			int fd = (int)arg1;
			int result = close(fd);
			cpu->x[10] = (uint32_t)result;
			break;
		}

		case SYS_brk: {
			cpu->x[10] = -ENOMEM;
			break;
		}

		case SYS_fstat: {
			int fd = (int)arg1;
			struct stat st;
			int result = fstat(fd, &st);

			if (result == 0 && arg2 + sizeof(st) <= mem->size) {
				size_t copy_size = sizeof(st) < 64 ? sizeof(st) : 64;
				memcpy(&mem->data[arg2], &st, copy_size);
			}

			cpu->x[10] = (uint32_t)result;
			break;
		}

		default:
			cpu->x[10] = -ENOSYS;
			break;
	}

	return CPU_OK;
}

static cpu_status_t execute_load(cpu_t *cpu, memory_t *mem, instruction_t *instr, uint32_t *result) {
	uint32_t addr = reg_read(cpu, instr->rs1) + instr->imm;
	uint32_t value;
	memory_status_t status;

	switch (instr->funct3) {
		case 0x0:
			status = memory_read8(mem, addr, (uint8_t*)&value);
			if (status != MEM_OK) return CPU_EXECUTION_ERROR;
			*result = sign_extend(value & 0xFF, 8);
			break;

		case 0x1:
			status = memory_read16(mem, addr, (uint16_t*)&value);
			if (status != MEM_OK) return CPU_EXECUTION_ERROR;
			*result = sign_extend(value & 0xFFFF, 16);
			break;

		case 0x2:
			status = memory_read32(mem, addr, &value);
			if (status != MEM_OK) return CPU_EXECUTION_ERROR;
			*result = value;
			break;

		case 0x4:
			status = memory_read8(mem, addr, (uint8_t*)&value);
			if (status != MEM_OK) return CPU_EXECUTION_ERROR;
			*result = value & 0xFF;
			break;

		case 0x5:
			status = memory_read16(mem, addr, (uint16_t*)&value);
			if (status != MEM_OK) return CPU_EXECUTION_ERROR;
			*result = value & 0xFFFF;
			break;

		default:
			return CPU_ILLEGAL_INSTRUCTION;
	}

	return CPU_OK;
}

static cpu_status_t execute_store(cpu_t *cpu, memory_t *mem, instruction_t *instr) {
	uint32_t addr = reg_read(cpu, instr->rs1) + instr->imm;
	uint32_t value = reg_read(cpu, instr->rs2);
	memory_status_t status;

	switch (instr->funct3) {
		case 0x0:
			status = memory_write8(mem, addr, value & 0xFF);
			break;

		case 0x1:
			status = memory_write16(mem, addr, value & 0xFFFF);
			break;

		case 0x2:
			status = memory_write32(mem, addr, value);
			break;

		default:
			return CPU_ILLEGAL_INSTRUCTION;
	}

	return (status == MEM_OK) ? CPU_OK : CPU_EXECUTION_ERROR;
}

static uint32_t execute_alu(uint32_t rs1_val, uint32_t rs2_val_or_imm, uint8_t funct3, uint8_t funct7, bool is_imm) {
	switch (funct3) {
		case 0x0:
			if (!is_imm && (funct7 & 0x20)) {
				return rs1_val - rs2_val_or_imm;
			}
			return rs1_val + rs2_val_or_imm;

		case 0x1:
			return rs1_val << (rs2_val_or_imm & 0x1F);

		case 0x2:
			return ((int32_t)rs1_val < (int32_t)rs2_val_or_imm) ? 1 : 0;

		case 0x3:
			return (rs1_val < rs2_val_or_imm) ? 1 : 0;

		case 0x4:
			return rs1_val ^ rs2_val_or_imm;

		case 0x5:
			if (!is_imm && (funct7 & 0x20)) {
				return (int32_t)rs1_val >> (rs2_val_or_imm & 0x1F);
			}
			return rs1_val >> (rs2_val_or_imm & 0x1F);

		case 0x6:
			return rs1_val | rs2_val_or_imm;

		case 0x7:
			return rs1_val & rs2_val_or_imm;

		default:
			return 0;
	}
}

static cpu_status_t execute_branch(cpu_t *cpu, instruction_t *instr) {
	uint32_t rs1_val = reg_read(cpu, instr->rs1);
	uint32_t rs2_val = reg_read(cpu, instr->rs2);
	bool take_branch = false;

	switch (instr->funct3) {
		case 0x0:
			take_branch = (rs1_val == rs2_val);
			break;

		case 0x1:
			take_branch = (rs1_val != rs2_val);
			break;

		case 0x4:
			take_branch = ((int32_t)rs1_val < (int32_t)rs2_val);
			break;

		case 0x5:
			take_branch = ((int32_t)rs1_val >= (int32_t)rs2_val);
			break;

		case 0x6:
			take_branch = (rs1_val < rs2_val);
			break;

		case 0x7:
			take_branch = (rs1_val >= rs2_val);
			break;

		default:
			return CPU_ILLEGAL_INSTRUCTION;
	}

	if (take_branch) {
		cpu->pc += instr->imm - 4;
	}

	return CPU_OK;
}

static cpu_status_t execute_system(cpu_t *cpu, memory_t *mem, instruction_t *instr) {
	switch (instr->imm & 0xFFF) {
		case 0x000:
			return handle_syscall(cpu, mem);

		case 0x001:
			fprintf(stderr, "Breakpoint at PC: 0x%08x\n", cpu->pc - 4);
			return CPU_OK;

		default:
			return CPU_ILLEGAL_INSTRUCTION;
	}
}

cpu_status_t cpu_execute(cpu_t *cpu, memory_t *mem, instruction_t *instr) {
	switch (instr->format) {
		case INSTR_R_TYPE: {
			uint32_t rs1_val = reg_read(cpu, instr->rs1);
			uint32_t rs2_val = reg_read(cpu, instr->rs2);
			uint32_t result = execute_alu(rs1_val, rs2_val, instr->funct3, instr->funct7, false);
			reg_write(cpu, instr->rd, result);
			break;
		}

		case INSTR_I_TYPE: {
			uint32_t rs1_val = reg_read(cpu, instr->rs1);

			switch (instr->opcode) {
				case 0x03: {
					uint32_t result;
					cpu_status_t status = execute_load(cpu, mem, instr, &result);
					if (status != CPU_OK) return status;
					reg_write(cpu, instr->rd, result);
					break;
				}

				case 0x13: {
					uint32_t result = execute_alu(rs1_val, instr->imm, instr->funct3, 0, true);
					reg_write(cpu, instr->rd, result);
					break;
				}

				case 0x67:
					reg_write(cpu, instr->rd, cpu->pc);
					cpu->pc = (rs1_val + instr->imm) & ~1;
					break;

				case 0x73:
					return execute_system(cpu, mem, instr);

				default:
					return CPU_ILLEGAL_INSTRUCTION;
			}
			break;
		}

		case INSTR_S_TYPE: {
			return execute_store(cpu, mem, instr);
		}

		case INSTR_B_TYPE: {
			return execute_branch(cpu, instr);
		}

		case INSTR_U_TYPE: {
			switch (instr->opcode) {
				case 0x37:
					reg_write(cpu, instr->rd, instr->imm);
					break;

				case 0x17:
					reg_write(cpu, instr->rd, cpu->pc + instr->imm - 4);
					break;

				default:
					return CPU_ILLEGAL_INSTRUCTION;
			}
			break;
		}

		case INSTR_J_TYPE:
			reg_write(cpu, instr->rd, cpu->pc);
			cpu->pc += instr->imm - 4;
			break;

		default:
			return CPU_DECODE_ERROR;
	}

	return CPU_OK;
}

cpu_status_t cpu_step(cpu_t *cpu, memory_t *mem) {
	if (!cpu->running) {
		return CPU_SYSCALL_EXIT;
	}

	uint32_t raw_instr;
	instruction_t decoded;

	cpu_status_t status = cpu_fetch(cpu, mem, &raw_instr);
	if (status != CPU_OK) {
		return status;
	}

	status = cpu_decode(raw_instr, &decoded);
	if (status != CPU_OK) {
		return status;
	}

	return cpu_execute(cpu, mem, &decoded);
}
