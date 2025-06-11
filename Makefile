# Compiler and flags
CC = clang
INCDIRS = -I$(CURDIR)/include
OPT = -O1

CFLAGS = -Wall -Wextra -g $(INCDIRS) $(OPT) --target=$(shell uname -m)-linux-gnu

# Directories
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin
TEST_DIR = tests
TEST_OBJ_DIR = $(TEST_DIR)/obj

# Files
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))
TEST_SRCS = $(wildcard $(TEST_DIR)/*.c)
TEST_OBJS = $(patsubst $(TEST_DIR)/%.c,$(TEST_OBJ_DIR)/%.o,$(TEST_SRCS))

# Executables
EXEC = $(BIN_DIR)/db
TEST_EXEC = $(BIN_DIR)/dbtest

.PHONY: all build-test test clean

all: $(EXEC)

# Main executable
$(EXEC): $(OBJS) | $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Test executable
build-test: $(TEST_EXEC)

$(TEST_EXEC): $(TEST_OBJS) | $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@

$(TEST_OBJ_DIR)/%.o: $(TEST_DIR)/%.c | $(TEST_OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Directory creation
$(BIN_DIR) $(OBJ_DIR) $(TEST_OBJ_DIR):
	mkdir -p $@

# Test command
test: build-test
	@$(TEST_EXEC)

clean-obj:
	rm -rf $(OBJ_DIR) $(TEST_OBJ_DIR)

clean-test:
	rm -rf $(TEST_EXEC) $(TEST_OBJ_DIR)

clean: clean-obj clean-test
	rm -rf $(BIN_DIR)
	@echo "All build artifacts removed"

# Handle arguments
%:
	@:
