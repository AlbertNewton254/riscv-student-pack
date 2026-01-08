# Makefile for RISC-V Tests (Unit and Integration)
CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -O2 -g
INCLUDES = -I../assembler/include -I../emulator/include

# Assembler source files
ASSEMBLER_SRCS = ../assembler/src/adjust_labels.cpp \
                 ../assembler/src/constructor.cpp \
                 ../assembler/src/encode.cpp \
                 ../assembler/src/expand_pseudoinstruction.cpp \
                 ../assembler/src/first_pass.cpp \
                 ../assembler/src/second_pass.cpp \
                 ../assembler/src/utils.cpp

# Emulator source files
EMULATOR_SRCS = ../emulator/src/cpu.cpp \
                ../emulator/src/instructions.cpp \
                ../emulator/src/memory.cpp

# Test source files
TEST_ASSEMBLER_SRC = assembler/test_assembler.cpp
TEST_EMULATOR_SRC = emulator/test_emulator.cpp
TEST_INTEGRATION_SRC = integration/test_integration.cpp

# Object files
ASSEMBLER_OBJS = $(ASSEMBLER_SRCS:.cpp=.o)
EMULATOR_OBJS = $(EMULATOR_SRCS:.cpp=.o)

TEST_ASSEMBLER_OBJ = $(TEST_ASSEMBLER_SRC:.cpp=.o)
TEST_EMULATOR_OBJ = $(TEST_EMULATOR_SRC:.cpp=.o)
TEST_INTEGRATION_OBJ = $(TEST_INTEGRATION_SRC:.cpp=.o)

# Target executables
TEST_ASSEMBLER = test_assembler
TEST_EMULATOR = test_emulator
TEST_INTEGRATION = test_integration

ALL_TESTS = $(TEST_ASSEMBLER) $(TEST_EMULATOR) $(TEST_INTEGRATION)

.PHONY: all test run_test_assembler run_test_emulator run_test_integration clean

all: $(ALL_TESTS)

# Assembler unit test
$(TEST_ASSEMBLER): $(TEST_ASSEMBLER_OBJ) $(ASSEMBLER_OBJS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^

# Emulator unit test
$(TEST_EMULATOR): $(TEST_EMULATOR_OBJ) $(EMULATOR_OBJS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^

# Integration test
$(TEST_INTEGRATION): $(TEST_INTEGRATION_OBJ) $(ASSEMBLER_OBJS) $(EMULATOR_OBJS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^

# Compile .cpp files to .o files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Run all tests
test: $(ALL_TESTS)
	@echo "=== Running assembler unit tests ==="
	./$(TEST_ASSEMBLER)
	@echo ""
	@echo "=== Running emulator unit tests ==="
	./$(TEST_EMULATOR)
	@echo ""
	@echo "=== Running integration tests ==="
	./$(TEST_INTEGRATION)

run_test_assembler: $(TEST_ASSEMBLER)
	./$(TEST_ASSEMBLER)

run_test_emulator: $(TEST_EMULATOR)
	./$(TEST_EMULATOR)

run_test_integration: $(TEST_INTEGRATION)
	./$(TEST_INTEGRATION)

clean:
	rm -f $(ALL_TESTS) $(TEST_ASSEMBLER_OBJ) $(TEST_EMULATOR_OBJ) $(TEST_INTEGRATION_OBJ) $(ASSEMBLER_OBJS) $(EMULATOR_OBJS)
