# Makefile for RISC-V Emulator

# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c99 -O2 -g
INCLUDES = -I./include

# Source files (relative to src directory)
SRC_DIR = src
SRC_C = $(SRC_DIR)/cpu.c $(SRC_DIR)/instructions.c $(SRC_DIR)/memory.c
SRC_MAIN = $(SRC_DIR)/main.c
SRC_TEST = tests/test_emulator.c

# Object files
OBJ = $(SRC_C:.c=.o)
OBJ_MAIN = $(SRC_MAIN:.c=.o)
OBJ_TEST = $(SRC_TEST:.c=.o)

# Executable names
EXEC = riscv_emulator
TEST_EXEC = test_emulator

# Default target
all: $(EXEC)

# Main emulator executable
$(EXEC): $(OBJ) $(OBJ_MAIN)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^

# Test executable
$(TEST_EXEC): $(OBJ) $(OBJ_TEST)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^

# Pattern rule for compiling .c files
%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Specific dependencies with correct paths
$(SRC_DIR)/cpu.o: $(SRC_DIR)/cpu.c include/cpu.h include/memory.h include/instructions.h
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(SRC_DIR)/instructions.o: $(SRC_DIR)/instructions.c include/instructions.h include/cpu.h
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(SRC_DIR)/memory.o: $(SRC_DIR)/memory.c include/memory.h
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(SRC_DIR)/main.o: $(SRC_DIR)/main.c include/cpu.h include/memory.h
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

tests/test_emulator.o: tests/test_emulator.c include/cpu.h include/memory.h include/instructions.h
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Run tests
test: $(TEST_EXEC)
	./$(TEST_EXEC)

# Run emulator with sample program
run: $(EXEC)
	@if [ -z "$(PROGRAM)" ]; then \
		echo "Usage: PROGRAM=program.bin make run"; \
		echo "Example: PROGRAM=output.bin make run"; \
		exit 1; \
	fi
	./$(EXEC) $(PROGRAM)

# Clean build files
clean:
	rm -f $(EXEC) $(TEST_EXEC) *.o $(SRC_DIR)/*.o tests/*.o

# Clean soft (keep executables)
clean-soft:
	rm -f *.o $(SRC_DIR)/*.o tests/*.o

# Format code (optional - requires clang-format)
format:
	find . -name "*.c" -o -name "*.h" | xargs clang-format -i

# Static analysis (optional - requires cppcheck)
analyze:
	cppcheck --enable=all --suppress=missingIncludeSystem .

# Debug build
debug: CFLAGS += -DDEBUG -O0
debug: clean $(EXEC)

# Release build (optimized)
release: CFLAGS += -O3 -DNDEBUG
release: clean $(EXEC)

# Help
help:
	@echo "Available targets:"
	@echo "  all       - Build the emulator (default)"
	@echo "  test      - Build and run unit tests"
	@echo "  run       - Run the emulator with PROGRAM variable"
	@echo "  clean     - Remove build artifacts"
	@echo "  clean-soft - Remove object files only (keep executables)"
	@echo "  format    - Format source code"
	@echo "  analyze   - Run static analysis"
	@echo "  debug     - Build with debug symbols"
	@echo "  release   - Build optimized release version"
	@echo "  help      - Show this help message"
	@echo ""
	@echo "Usage examples:"
	@echo "  make                         # Build emulator"
	@echo "  make test                    # Build and run tests"
	@echo "  PROGRAM=program.bin make run # Run with specific binary"
	@echo "  make clean                   # Clean build files"

.PHONY: all test run clean clean-soft format analyze debug release help
