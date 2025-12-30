# RISC-V Emulator Makefile
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g -I./include
LDFLAGS = 

# Source files
SRC_DIR = src
SRCS = $(SRC_DIR)/cpu.c \
	   $(SRC_DIR)/memory.c \
	   $(SRC_DIR)/instructions.c \
	   $(SRC_DIR)/main.c

# Object files
OBJS = $(SRCS:.c=.o)

# Executables
TARGET = riscv_emulator
TEST_TARGET = tests/test_emulator

# Include directory
INCLUDE_DIR = include

.PHONY: all clean clean-soft test

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Compile all .c files in src/ to .o files
$(SRC_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Test target
test: $(TEST_TARGET)
	@echo "Running emulator tests..."
	@./$(TEST_TARGET)

$(TEST_TARGET): tests/test_emulator.c $(filter-out $(SRC_DIR)/main.o, $(OBJS))
	$(CC) $(CFLAGS) -I./include -o $@ $^ $(LDFLAGS)

# Clean everything
clean:
	rm -f $(TARGET) $(TEST_TARGET) 2>/dev/null || true
	rm -f $(SRC_DIR)/*.o 2>/dev/null || true
	rm -f *.o 2>/dev/null || true
	rm -f tests/*.bin tests/*.out 2>/dev/null || true

# Clean object files only (keep main binary)
clean-soft:
	rm -f $(TEST_TARGET) 2>/dev/null || true
	rm -f $(SRC_DIR)/*.o 2>/dev/null || true
	rm -f *.o 2>/dev/null || true
	rm -f tests/*.bin tests/*.out 2>/dev/null || true
