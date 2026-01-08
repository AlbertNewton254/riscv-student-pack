/* cpu.cpp */
#include "cpu.hpp"
#include "memory.hpp"
#include "instructions.hpp"
#include <cstdlib>
#include <memory>
#include <cstdio>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <cerrno>
#include <cstring>

CPU::CPU() {
	for (int i = 0; i < 32; i++) {
		x[i] = 0;
	}
	pc = 0;
	running = true;
	debug_mode = false;

	x[2] = STACK_TOP;
}

bool CPU::is_running() const {
	return running;
}

void CPU::set_debug_mode(bool enable) {
	debug_mode = enable;
}

/* Helper function to get instruction name */
static const char* get_instruction_name(uint8_t opcode, uint8_t funct3, uint8_t funct7) {
	switch (opcode) {
		case 0x33: /* R-type ALU */
			if (funct7 == 0x00) {
				switch (funct3) {
					case 0x0: return "add";
					case 0x4: return "xor";
					case 0x6: return "or";
					case 0x7: return "and";
					case 0x1: return "sll";
					case 0x5: return "srl";
					case 0x2: return "slt";
					case 0x3: return "sltu";
				}
			} else if (funct7 == 0x20) {
				if (funct3 == 0x0) return "sub";
				if (funct3 == 0x5) return "sra";
			}
			return "alu-r";
		case 0x13: /* I-type ALU */
			switch (funct3) {
				case 0x0: return "addi";
				case 0x4: return "xori";
				case 0x6: return "ori";
				case 0x7: return "andi";
				case 0x1: return "slli";
				case 0x5: return (funct7 == 0x00) ? "srli" : "srai";
				case 0x2: return "slti";
				case 0x3: return "sltiu";
			}
			return "alu-i";
		case 0x03: /* Load */
			switch (funct3) {
				case 0x0: return "lb";
				case 0x1: return "lh";
				case 0x2: return "lw";
				case 0x4: return "lbu";
				case 0x5: return "lhu";
			}
			return "load";
		case 0x23: /* Store */
			switch (funct3) {
				case 0x0: return "sb";
				case 0x1: return "sh";
				case 0x2: return "sw";
			}
			return "store";
		case 0x63: /* Branch */
			switch (funct3) {
				case 0x0: return "beq";
				case 0x1: return "bne";
				case 0x4: return "blt";
				case 0x5: return "bge";
				case 0x6: return "bltu";
				case 0x7: return "bgeu";
			}
			return "branch";
		case 0x6f: return "jal";
		case 0x67: return "jalr";
		case 0x37: return "lui";
		case 0x17: return "auipc";
		case 0x73: return (funct3 == 0x0) ? "ecall" : "system";
		default: return "unknown";
	}
}

uint32_t CPU::get_pc() const {
	return pc;
}

void CPU::set_pc(uint32_t value) {
	pc = value;
}

uint32_t CPU::get_register(uint8_t reg) const {
	if (reg >= 32) return 0;
	return (reg == 0) ? 0 : x[reg];
}

void CPU::set_register(uint8_t reg, uint32_t value) {
	if (reg > 0 && reg < 32) {
		x[reg] = value;
	}
}

uint32_t CPU::reg_read(uint8_t reg) {
	return (reg == 0) ? 0 : x[reg];
}

void CPU::reg_write(uint8_t reg, uint32_t value) {
	if (reg != 0) {
		x[reg] = value;
	}
}

cpu_status_t CPU::fetch(Memory *mem, uint32_t *instruction) {
	memory_status_t status = mem->read32(pc, instruction);

	if (status == MEM_OK) {
		pc += 4;
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

cpu_status_t CPU::handle_syscall(Memory *mem) {
	uint32_t syscall_num = x[17];
	uint32_t arg1 = x[10];
	uint32_t arg2 = x[11];
	uint32_t arg3 = x[12];

	switch (syscall_num) {
		case SYS_exit: {
			running = false;
			return CPU_SYSCALL_EXIT;
		}

		case SYS_write: {
			int fd = (int)arg1;
			uint32_t buf_addr = arg2;
			size_t count = (size_t)arg3;

			if (buf_addr + count > mem->get_size()) {
				x[10] = -1;
				break;
			}

			ssize_t result = write(fd, &mem->get_data()[buf_addr], count);
			x[10] = (uint32_t)result;
			break;
		}

		case SYS_read: {
			int fd = (int)arg1;
			uint32_t buf_addr = arg2;
			size_t count = (size_t)arg3;

			if (buf_addr + count > mem->get_size()) {
				x[10] = -1;
				break;
			}

			ssize_t result = read(fd, &mem->get_data()[buf_addr], count);
			x[10] = (uint32_t)result;
			break;
		}

		case SYS_openat: {
			uint32_t path_addr = arg2;
			int flags = (int)arg3;
			mode_t mode = (mode_t)arg3;

			char path[256];
			int i;
			for (i = 0; i < (int)sizeof(path) - 1; i++) {
				if (path_addr + i >= mem->get_size()) break;
				path[i] = mem->get_data()[path_addr + i];
				if (path[i] == '\0') break;
			}
			path[i] = '\0';

			int result = open(path, flags, mode);
			x[10] = (uint32_t)result;
			break;
		}

		case SYS_close: {
			int fd = (int)arg1;
			int result = close(fd);
			x[10] = (uint32_t)result;
			break;
		}

		case SYS_brk: {
			x[10] = -ENOMEM;
			break;
		}

		case SYS_fstat: {
			int fd = (int)arg1;
			struct stat st;
			int result = fstat(fd, &st);

			if (result == 0 && arg2 + sizeof(st) <= mem->get_size()) {
				size_t copy_size = sizeof(st) < 64 ? sizeof(st) : 64;
				std::memcpy(&mem->get_data()[arg2], &st, copy_size);
			}

			x[10] = (uint32_t)result;
			break;
		}

		default:
			x[10] = -ENOSYS;
			break;
	}

	return CPU_OK;
}

cpu_status_t CPU::execute_load(Memory *mem, Instruction *instr, uint32_t *result) {
	uint32_t addr = reg_read(instr->get_rs1()) + instr->get_imm();
	uint32_t value;
	memory_status_t status;

	switch (instr->get_funct3()) {
		case 0x0:
			status = mem->read8(addr, (uint8_t*)&value);
			if (status != MEM_OK) return CPU_EXECUTION_ERROR;
			*result = sign_extend(value & 0xFF, 8);
			break;

		case 0x1:
			status = mem->read16(addr, (uint16_t*)&value);
			if (status != MEM_OK) return CPU_EXECUTION_ERROR;
			*result = sign_extend(value & 0xFFFF, 16);
			break;

		case 0x2:
			status = mem->read32(addr, &value);
			if (status != MEM_OK) return CPU_EXECUTION_ERROR;
			*result = value;
			break;

		case 0x4:
			status = mem->read8(addr, (uint8_t*)&value);
			if (status != MEM_OK) return CPU_EXECUTION_ERROR;
			*result = value & 0xFF;
			break;

		case 0x5:
			status = mem->read16(addr, (uint16_t*)&value);
			if (status != MEM_OK) return CPU_EXECUTION_ERROR;
			*result = value & 0xFFFF;
			break;

		default:
			return CPU_ILLEGAL_INSTRUCTION;
	}

	return CPU_OK;
}

cpu_status_t CPU::execute_store(Memory *mem, Instruction *instr) {
	uint32_t addr = reg_read(instr->get_rs1()) + instr->get_imm();
	uint32_t value = reg_read(instr->get_rs2());
	memory_status_t status;

	switch (instr->get_funct3()) {
		case 0x0:
			status = mem->write8(addr, value & 0xFF);
			break;

		case 0x1:
			status = mem->write16(addr, value & 0xFFFF);
			break;

		case 0x2:
			status = mem->write32(addr, value);
			break;

		default:
			return CPU_ILLEGAL_INSTRUCTION;
	}

	return (status == MEM_OK) ? CPU_OK : CPU_EXECUTION_ERROR;
}

uint32_t CPU::execute_alu(uint32_t rs1_val, uint32_t rs2_val_or_imm, uint8_t funct3, uint8_t funct7, bool is_imm) {
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

cpu_status_t CPU::execute_branch(Instruction *instr) {
	uint32_t rs1_val = reg_read(instr->get_rs1());
	uint32_t rs2_val = reg_read(instr->get_rs2());
	bool take_branch = false;

	switch (instr->get_funct3()) {
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
		pc += instr->get_imm() - 4;
	}

	return CPU_OK;
}

cpu_status_t CPU::execute_system(Memory *mem, Instruction *instr) {
	switch (instr->get_imm() & 0xFFF) {
		case 0x000:
			return handle_syscall(mem);

		case 0x001:
			std::fprintf(stderr, "Breakpoint at PC: 0x%08x\n", pc - 4);
			return CPU_OK;

		default:
			return CPU_ILLEGAL_INSTRUCTION;
	}
}

cpu_status_t CPU::execute(Memory *mem, Instruction *instr) {
	switch (instr->get_format()) {
		case INSTR_R_TYPE: {
			uint32_t rs1_val = reg_read(instr->get_rs1());
			uint32_t rs2_val = reg_read(instr->get_rs2());
			uint32_t result = execute_alu(rs1_val, rs2_val, instr->get_funct3(), instr->get_funct7(), false);
			reg_write(instr->get_rd(), result);
			break;
		}

		case INSTR_I_TYPE: {
			uint32_t rs1_val = reg_read(instr->get_rs1());

			switch (instr->get_opcode()) {
				case 0x03: {
					uint32_t result;
					cpu_status_t status = execute_load(mem, instr, &result);
					if (status != CPU_OK) return status;
					reg_write(instr->get_rd(), result);
					break;
				}

				case 0x13: {
					uint32_t result = execute_alu(rs1_val, instr->get_imm(), instr->get_funct3(), 0, true);
					reg_write(instr->get_rd(), result);
					break;
				}

				case 0x67:
					reg_write(instr->get_rd(), pc);
					pc = (rs1_val + instr->get_imm()) & ~1;
					break;

				case 0x73:
					return execute_system(mem, instr);

				default:
					return CPU_ILLEGAL_INSTRUCTION;
			}
			break;
		}

		case INSTR_S_TYPE: {
			return execute_store(mem, instr);
		}

		case INSTR_B_TYPE: {
			return execute_branch(instr);
		}

		case INSTR_U_TYPE: {
			switch (instr->get_opcode()) {
				case 0x37:
					reg_write(instr->get_rd(), instr->get_imm());
					break;

				case 0x17:
					reg_write(instr->get_rd(), pc + instr->get_imm() - 4);
					break;

				default:
					return CPU_ILLEGAL_INSTRUCTION;
			}
			break;
		}

		case INSTR_J_TYPE:
			reg_write(instr->get_rd(), pc);
			pc += instr->get_imm() - 4;
			break;

		default:
			return CPU_DECODE_ERROR;
	}

	return CPU_OK;
}

cpu_status_t CPU::step(Memory *mem) {
	if (!running) {
		return CPU_SYSCALL_EXIT;
	}

	uint32_t raw_instr;
	Instruction decoded;

	/* FETCH */
	if (debug_mode) {
		std::printf("[FETCH] PC=0x%08x\n", pc);
	}

	cpu_status_t status = fetch(mem, &raw_instr);
	if (status != CPU_OK) {
		if (debug_mode) {
			std::printf("  FETCH ERROR: status=%d\n", status);
		}
		return status;
	}

	if (debug_mode) {
		std::printf("  Instruction: 0x%08x\n", raw_instr);
	}

	/* DECODE */
	if (!decoded.decode(raw_instr)) {
		if (debug_mode) {
			std::printf("  DECODE ERROR\n");
		}
		return CPU_DECODE_ERROR;
	}

	if (debug_mode) {
		const char* instr_name = get_instruction_name(decoded.get_opcode(),
			decoded.get_funct3(), decoded.get_funct7());
		std::printf("[DECODE] %s (opcode=0x%02x", instr_name, decoded.get_opcode());

		switch (decoded.get_format()) {
			case INSTR_R_TYPE:
				std::printf(", rd=x%d, rs1=x%d, rs2=x%d, funct3=0x%x, funct7=0x%x)\n",
					decoded.get_rd(), decoded.get_rs1(), decoded.get_rs2(),
					decoded.get_funct3(), decoded.get_funct7());
				break;
			case INSTR_I_TYPE:
				std::printf(", rd=x%d, rs1=x%d, imm=%d)\n",
					decoded.get_rd(), decoded.get_rs1(), decoded.get_imm());
				break;
			case INSTR_S_TYPE:
				std::printf(", rs1=x%d, rs2=x%d, imm=%d)\n",
					decoded.get_rs1(), decoded.get_rs2(), decoded.get_imm());
				break;
			case INSTR_B_TYPE:
				std::printf(", rs1=x%d, rs2=x%d, imm=%d, target=0x%08x)\n",
					decoded.get_rs1(), decoded.get_rs2(), decoded.get_imm(),
					pc + decoded.get_imm());
				break;
			case INSTR_U_TYPE:
				std::printf(", rd=x%d, imm=0x%x)\n",
					decoded.get_rd(), decoded.get_imm());
				break;
			case INSTR_J_TYPE:
				std::printf(", rd=x%d, imm=%d, target=0x%08x)\n",
					decoded.get_rd(), decoded.get_imm(), pc + decoded.get_imm());
				break;
		}
	}

	/* EXECUTE */
	if (debug_mode) {
		std::printf("[EXECUTE] ");
	}

	status = execute(mem, &decoded);

	if (debug_mode) {
		if (status == CPU_OK) {
			std::printf("OK, next_pc=0x%08x\n\n", pc);
		} else {
			std::printf("ERROR: status=%d\n\n", status);
		}
	}

	return status;
}