# Makefile for RISC-V Emulator

# Compiler and flags
CXX = g++
CXXFLAGS = -Wall -Wextra -Werror -std=c++17 -O2 -g
INCLUDES = -I./include

# Source files (relative to src directory)
SRC_DIR = src
SRC_CPP = $(SRC_DIR)/cpu.cpp $(SRC_DIR)/instructions.cpp $(SRC_DIR)/memory.cpp $(SRC_DIR)/emulator.cpp
SRC_MAIN = $(SRC_DIR)/main.cpp

# Object files
OBJ = $(SRC_CPP:.cpp=.o)
OBJ_MAIN = $(SRC_MAIN:.cpp=.o)

# Executable names
EXEC = riscv_emulator

# Default target
all: $(EXEC)

# Main emulator executable
$(EXEC): $(OBJ) $(OBJ_MAIN)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^

# Pattern rule for compiling .cpp files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Specific dependencies with correct paths
$(SRC_DIR)/cpu.o: $(SRC_DIR)/cpu.cpp include/cpu.hpp include/memory.hpp include/instructions.hpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(SRC_DIR)/instructions.o: $(SRC_DIR)/instructions.cpp include/instructions.hpp include/cpu.hpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(SRC_DIR)/memory.o: $(SRC_DIR)/memory.cpp include/memory.hpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(SRC_DIR)/emulator.o: $(SRC_DIR)/emulator.cpp include/emulator.hpp include/cpu.hpp include/memory.hpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(SRC_DIR)/main.o: $(SRC_DIR)/main.cpp include/emulator.hpp include/cpu.hpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Run emulator with sample program
run: $(EXEC)
	@if [ -z "$(PROGRAM)" ]; then \
		echo "Usage: PROGRAM=program.bin make run"; \
		echo "Example: PROGRAM=output.bin make run"; \
		exit 1; \
	fi
	./$(EXEC) $(PROGRAM)

# Clean everything (including executables)
clean:
	rm -f $(EXEC) *.o $(SRC_DIR)/*.o

# Clean object files only (keep executables for fast rebuilds)
clean-soft:
	rm -f *.o $(SRC_DIR)/*.o

# Format code (optional - requires clang-format)
format:
	find . -name "*.cpp" -o -name "*.hpp" | xargs clang-format -i

# Static analysis (optional - requires cppcheck)
analyze:
	cppcheck --enable=all --suppress=missingIncludeSystem .

# Debug build
debug: CXXFLAGS += -DDEBUG -O0
debug: clean $(EXEC)

# Release build (optimized)
release: CXXFLAGS += -O3 -DNDEBUG
release: clean $(EXEC)

# Help
help:
	@echo "Available targets:"
	@echo "  all       - Build the emulator (default)"
	@echo "  run       - Run the emulator with PROGRAM variable"
	@echo "  clean     - Remove everything (including executables)"
	@echo "  clean-soft - Remove object files only (keep executables for fast rebuilds)"
	@echo "  format    - Format source code"
	@echo "  analyze   - Run static analysis"
	@echo "  debug     - Build with debug symbols"
	@echo "  release   - Build optimized release version"
	@echo "  help      - Show this help message"
	@echo ""
	@echo "Usage examples:"
	@echo "  make                         # Build emulator"
	@echo "  PROGRAM=program.bin make run # Run with specific binary"
	@echo "  make clean                   # Clean build files"
	@echo ""
	@echo "Tests moved to: ../tests/"
	@echo "Run: cd ../tests && make test_emulator"

.PHONY: all run clean clean-soft format analyze debug release help