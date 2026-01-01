# Root Makefile for RISC-V Assembler and Emulator project
.PHONY: all assembler emulator clean clean-all test test-assembler test-emulator unit-tests help

# Default target
all: assembler emulator

# Build only assembler
assembler:
	@echo "Building RISC-V Assembler..."
	@cd assembler && $(MAKE) all

# Build only emulator
emulator:
	@echo "Building RISC-V Emulator..."
	@cd emulator && $(MAKE) all

# Clean everything
clean:
	@echo "Cleaning build artifacts..."
	@cd assembler && $(MAKE) clean 2>/dev/null || true
	@cd emulator && $(MAKE) clean 2>/dev/null || true
	@rm -f *.bin *.s 2>/dev/null || true

# Deep clean (including executables)
clean-all:
	@echo "Cleaning all files including executables..."
	@cd assembler && $(MAKE) clean 2>/dev/null || true
	@cd emulator && $(MAKE) clean 2>/dev/null || true
	@rm -f assembler/riscv_assembler 2>/dev/null || true
	@rm -f assembler/test_assembler 2>/dev/null || true
	@rm -f emulator/riscv_emulator 2>/dev/null || true
	@rm -f emulator/test_emulator 2>/dev/null || true
	@rm -f *.bin *.s 2>/dev/null || true
	@find . -name "*.o" -delete 2>/dev/null || true

# Run tests
test: test-assembler test-emulator

unit-tests: test

test-assembler:
	@echo "Testing RISC-V Assembler..."
	@cd assembler && $(MAKE) test

test-emulator:
	@echo "Testing RISC-V Emulator..."
	@cd emulator && $(MAKE) test

# Format code
format:
	@echo "Formatting assembler code..."
	@cd assembler && $(MAKE) format 2>/dev/null || true
	@echo "Formatting emulator code..."
	@cd emulator && $(MAKE) format 2>/dev/null || true

# Static analysis
analyze:
	@echo "Analyzing assembler code..."
	@cd assembler && $(MAKE) analyze 2>/dev/null || true
	@echo "Analyzing emulator code..."
	@cd emulator && $(MAKE) analyze 2>/dev/null || true

# Debug builds
debug:
	@cd assembler && $(MAKE) debug
	@cd emulator && $(MAKE) debug

# Release builds
release:
	@cd assembler && $(MAKE) release
	@cd emulator && $(MAKE) release

# Help
help:
	@echo "RISC-V Assembler & Emulator Project"
	@echo "====================================="
	@echo ""
	@echo "Available targets:"
	@echo "  all           - Build both assembler and emulator (default)"
	@echo "  assembler     - Build only the assembler"
	@echo "  emulator      - Build only the emulator"
	@echo "  clean         - Clean build artifacts"
	@echo "  clean-all     - Clean everything including executables"
	@echo "  test          - Run all tests"
	@echo "  test-assembler - Run assembler tests"
	@echo "  test-emulator  - Run emulator tests"
	@echo "  format        - Format source code"
	@echo "  analyze       - Run static analysis"
	@echo "  debug         - Build with debug symbols"
	@echo "  release       - Build optimized release version"
	@echo "  help          - Show this help message"
	@echo ""
	@echo "Examples:"
	@echo "  make all                     # Build everything"
	@echo "  make assembler && make test-assembler # Build and test assembler"
	@echo "  make clean-all               # Clean all build files"
	@echo ""
	@echo "Project Structure:"
	@echo "  assembler/    - RISC-V assembler implementation"
	@echo "  emulator/     - RISC-V emulator implementation"
	@echo "  unit_tests.sh - Test runner script"
