CC=gcc
ICDIRS=-I
OPT=-O1
CFLAGS=-Wall -Wextra -g $(INCDIRS) $(OPT)

CFILES=src/db.c
BINARY=bin/db

all: $(BINARY)

$(BINARY):
	$(CC) $(CFLAGS) $(CFILES) -o $@

run:
	$(BINARY)

clean:
	rm -f $(BINARY)
