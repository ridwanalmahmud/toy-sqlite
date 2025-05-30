CC = gcc
INCDIRS = -I.
OPT = -O1
CFLAGS = -Wall -Wextra -g $(INCDIRS) $(OPT)

CFILES = src/db.c
TESTFILES = test/test.c
BINARY = bin/db
TESTBINARY = test/dbtest

all: $(BINARY)

$(BINARY): $(CFILES)
	@mkdir -p bin
	$(CC) $(CFLAGS) $(CFILES) -o $@

# Run with arguments: `make run arg1 arg2`
run: $(BINARY)
	@$(BINARY) $(filter-out $@,$(MAKECMDGOALS))

# Prevent Make from interpreting args as targets
%:
	@:

build-test: $(TESTBINARY)

$(TESTBINARY): $(BINARY) $(TESTFILES)
	@mkdir -p test
	$(CC) $(CFLAGS) $(TESTFILES) -o $@

run-test: $(TESTBINARY)
	@$(TESTBINARY) $(filter-out $@,$(MAKECMDGOALS))

test: build-test run-test

clean:
	rm -f $(BINARY) $(TESTBINARY)

.PHONY: all run build-test run-test test clean
