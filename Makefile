# Root Makefile for RISC-V Assembler and Emulator project
.PHONY: all assembler emulator clean clean-all test test-all test-assembler test-emulator test-integration unit-tests integration-tests format analyze debug release help

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

# Clean object files (keep executables for fast rebuilds)
clean:
	@echo "Cleaning object files..."
	@cd assembler && $(MAKE) clean-soft 2>/dev/null || true
	@cd emulator && $(MAKE) clean-soft 2>/dev/null || true
	@cd tests && $(MAKE) clean 2>/dev/null || true
	@rm -f *.bin *.s 2>/dev/null || true

# Deep clean (remove everything including executables)
clean-all:
	@echo "Cleaning all files including executables..."
	@cd assembler && $(MAKE) clean 2>/dev/null || true
	@cd emulator && $(MAKE) clean 2>/dev/null || true
	@cd tests && $(MAKE) clean 2>/dev/null || true
	@rm -f *.bin *.s 2>/dev/null || true

# Run all tests via unified test Makefile
test: test-all

test-all:
	@echo "Running all tests (assembler, emulator, integration)..."
	@cd tests && $(MAKE) test

# Run unit tests only (assembler + emulator)
unit-tests:
	@echo "Running unit tests (assembler + emulator)..."
	@cd tests && $(MAKE) run_test_assembler run_test_emulator

# Run assembler tests
test-assembler:
	@echo "Running assembler unit tests..."
	@cd tests && $(MAKE) run_test_assembler

# Run emulator tests
test-emulator:
	@echo "Running emulator unit tests..."
	@cd tests && $(MAKE) run_test_emulator

# Run integration tests
test-integration:
	@echo "Running integration tests..."
	@cd tests && $(MAKE) run_test_integration

# Run all tests (shell script)
integration-tests:
	@echo "Running complete integration test suite..."
	@cd tests && ./all_tests.sh

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
	@echo "  all                - Build both assembler and emulator (default)"
	@echo "  assembler          - Build only the assembler"
	@echo "  emulator           - Build only the emulator"
	@echo "  clean              - Clean object files (keep executables)"
	@echo "  clean-all          - Clean everything including executables"
	@echo ""
	@echo "  test               - Run all tests (assembler + emulator + integration)"
	@echo "  test-all           - Run all tests"
	@echo "  unit-tests         - Run unit tests (assembler + emulator only)"
	@echo "  test-assembler     - Run assembler unit tests"
	@echo "  test-emulator      - Run emulator unit tests"
	@echo "  test-integration   - Run integration tests"
	@echo "  integration-tests  - Run complete test suite (with cleanup)"
	@echo ""
	@echo "  format             - Format source code"
	@echo "  analyze            - Run static analysis"
	@echo "  debug              - Build with debug symbols"
	@echo "  release            - Build optimized release version"
	@echo "  help               - Show this help message"
	@echo ""
	@echo "Examples:"
	@echo "  make                           # Build everything"
	@echo "  make test                      # Run all tests"
	@echo "  make unit-tests                # Run unit tests only"
	@echo "  make test-assembler            # Run assembler tests"
	@echo "  make integration-tests         # Run complete test suite"
	@echo "  make clean-all                 # Clean all build files"
	@echo ""
	@echo "Test Scripts (in tests/ folder):"
	@echo "  ./all_tests.sh                 # Complete test suite"
	@echo "  ./unit_tests.sh                # Unit tests only"
	@echo "  ./integration_tests.sh         # Integration tests only"
	@echo ""
	@echo "Project Structure:"
	@echo "  assembler/        - RISC-V assembler implementation"
	@echo "  emulator/         - RISC-V emulator implementation"
	@echo "  tests/            - Test files and test infrastructure"
	@echo "  tests/assembler/  - Assembler unit tests"
	@echo "  tests/emulator/   - Emulator unit tests"
	@echo "  tests/integration/- Integration tests"