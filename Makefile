# Compiler and flags
CC = clang
INCDIRS = -Iinclude
OPT = -O1

ifndef TARGET
    ARCH := $(shell uname -m)
    ifeq ($(ARCH),aarch64)
        TARGET := aarch64-linux-gnu
    else
        TARGET := x86_64-linux-gnu
    endif
endif

CFLAGS = -Wall -Wextra -g $(INCDIRS) $(OPT) --target=$(TARGET)

# Directories
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin
INC_DIR = include
TEST_DIR = tests

# Executables
EXEC = $(BIN_DIR)/db
TEST_EXEC = $(TEST_DIR)/dbtest

# Source files
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))

# Phony targets
.PHONY: all run test build-test clean clean-obj

# Default target
all: $(EXEC)

# Rule to build main executable only if it doesn't exist
$(EXEC): | $(BIN_DIR)
	@if [ ! -f "$(EXEC)" ]; then \
		echo "Building database executable..."; \
		mkdir -p $(OBJ_DIR); \
		for src in $(SRCS); do \
			obj=$(OBJ_DIR)/$$(basename $${src%.c}.o); \
			$(CC) $(CFLAGS) -c $$src -o $$obj; \
		done; \
		$(CC) $(CFLAGS) $(OBJS) -o $(EXEC); \
	else \
		echo "Database executable already exists"; \
	fi

# Rule to build test executable only if it doesn't exist
$(TEST_EXEC): | $(TEST_DIR)
	@if [ ! -f "$(TEST_EXEC)" ]; then \
		echo "Building test executable..."; \
		$(CC) $(CFLAGS) $(wildcard $(TEST_DIR)/*.c) -o $(TEST_EXEC); \
	else \
		echo "Test executable already exists"; \
	fi

# Directory creation
$(BIN_DIR) $(TEST_DIR):
	mkdir -p $@

# Run command - uses existing executable or builds if missing
run: $(EXEC)
	@$(EXEC) $(filter-out $@,$(MAKECMDGOALS))

# Test command - uses existing test executable or builds if missing
test: $(TEST_EXEC)
	@$(TEST_EXEC) $(filter-out $@,$(MAKECMDGOALS))

# Build test explicitly
build-test: $(TEST_EXEC)

# Clean object files only
clean-obj:
	rm -rf $(OBJ_DIR)
	@echo "Object files removed"

# Clean test files only
clean-test:
	rm -rf $(TEST_EXEC)
	rm -rf test.db
	@echo "Test files removed"

# Clean everything
clean: clean-obj clean-test
	rm -rf $(BIN_DIR) $(TEST_EXEC)
	@echo "All build artifacts removed"

# Handle arguments
%:
	@:
