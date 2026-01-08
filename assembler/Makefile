# Makefile for RISC-V Assembler

# Compiler and flags
CXX = g++
CXXFLAGS = -Wall -Wextra -Werror -std=c++17 -O2 -g
INCLUDES = -I./include

# Source files (relative to src directory)
SRC_DIR = src
SRC_CPP = $(SRC_DIR)/adjust_labels.cpp $(SRC_DIR)/constructor.cpp $(SRC_DIR)/encode.cpp $(SRC_DIR)/expand_pseudoinstruction.cpp \
        $(SRC_DIR)/first_pass.cpp $(SRC_DIR)/second_pass.cpp $(SRC_DIR)/utils.cpp
SRC_MAIN = $(SRC_DIR)/main.cpp

# Object files
OBJ = $(SRC_CPP:.cpp=.o)
OBJ_MAIN = $(SRC_MAIN:.cpp=.o)

# Executable names
EXEC = riscv_assembler

# Default target
all: $(EXEC)

# Main assembler executable
$(EXEC): $(OBJ) $(OBJ_MAIN)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^

# Compile .cpp files to .o files
%.o: %.cpp include/assembler.hpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Run assembler with sample assembly file
run: $(EXEC)
	@if [ -z "$(PROGRAM)" ]; then \
		echo "Usage: PROGRAM=input.s make run"; \
		echo "Example: PROGRAM=test.s make run"; \
		exit 1; \
	fi
	./$(EXEC) $(PROGRAM) output.bin

# Clean everything (including executables)
clean:
	rm -f $(EXEC) *.o $(SRC_DIR)/*.o output.bin

# Clean object files only (keep executables for fast rebuilds)
clean-soft:
	rm -f *.o $(SRC_DIR)/*.o output.bin

# Format code (optional - requires clang-format)
format:
	find . -name "*.cpp" -o -name "*.hpp" | xargs clang-format -i

# Static analysis (optional - requires cppcheck)
analyze:
	cppcheck --enable=all --suppress=missingIncludeSystem .

# Debug build
debug: CFLAGS += -DDEBUG -O0
debug: clean $(EXEC)

# Release build (optimized)
release: CFLAGS += -O3 -DNDEBUG
release: clean $(EXEC)

# Individual compilation for debugging
$(SRC_DIR)/adjust_labels.o: $(SRC_DIR)/adjust_labels.cpp include/assembler.hpp

$(SRC_DIR)/encode.o: $(SRC_DIR)/encode.cpp include/assembler.hpp

$(SRC_DIR)/expand_pseudoinstruction.o: $(SRC_DIR)/expand_pseudoinstruction.cpp include/assembler.hpp

$(SRC_DIR)/first_pass.o: $(SRC_DIR)/first_pass.cpp include/assembler.hpp

$(SRC_DIR)/second_pass.o: $(SRC_DIR)/second_pass.cpp include/assembler.hpp

$(SRC_DIR)/utils.o: $(SRC_DIR)/utils.cpp include/assembler.hpp

$(SRC_DIR)/main.o: $(SRC_DIR)/main.cpp include/assembler.hpp

# Help
help:
	@echo "Available targets:"
	@echo "  all       - Build the assembler (default)"
	@echo "  run       - Run assembler with PROGRAM variable"
	@echo "  clean     - Remove everything (including executables)"
	@echo "  clean-soft - Remove object files only (keep executables for fast rebuilds)"
	@echo "  format    - Format source code"
	@echo "  analyze   - Run static analysis"
	@echo "  debug     - Build with debug symbols"
	@echo "  release   - Build optimized release version"
	@echo "  help      - Show this help message"
	@echo ""
	@echo "Usage examples:"
	@echo "  make                         # Build assembler"
	@echo "  PROGRAM=test.s make run      # Assemble test.s to output.bin"
	@echo "  make clean                   # Clean build files"
	@echo ""
	@echo "Tests moved to: ../tests/"
	@echo "Run: cd ../tests && make test_assembler"

.PHONY: all run clean clean-soft format analyze debug release help