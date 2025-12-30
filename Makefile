# Root Makefile for RISC-V Assembler and Emulator project
.PHONY: all assembler emulator clean test unit-tests

all:
	@echo "Building assembler..."
	@cd assembler && $(MAKE) all
	@echo "Building emulator..."
	@cd emulator && $(MAKE) all

assembler:
	@cd assembler && $(MAKE) all

emulator:
	@cd emulator && $(MAKE) all

clean:
	@echo "Cleaning object files and test executables..."
	@cd assembler && $(MAKE) clean || true
	@cd emulator && $(MAKE) clean || true
	@find . -name "*.o" -delete 2>/dev/null || true
	@find . -name "test_*" -type f -executable -delete 2>/dev/null || true
	@find . -name "*.bin" -delete 2>/dev/null || true
	@find . -name "*.s" -delete 2>/dev/null || true

test: unit-tests

unit-tests:
	@chmod +x unit_tests.sh
	@./unit_tests.sh

# Keep binaries when cleaning
clean-soft:
	@echo "Cleaning object files only (keeping binaries)..."
	@cd assembler && $(MAKE) clean-soft 2>/dev/null || true
	@cd emulator && $(MAKE) clean-soft 2>/dev/null || true
	@find . -name "*.o" -delete 2>/dev/null || true
	@find . -name "test_*" -type f -executable -delete 2>/dev/null || true
