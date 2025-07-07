CC=clang
INCDIR=include
SRCDIR=src
OBJDIR=obj
TESTDIR=tests
BUILDDIR=build
TESTOBJDIR=$(TESTDIR)/obj
CFLAGS=-Wall -Wextra -g -I$(INCDIR) -pedantic -pipe

SRCS=$(wildcard $(SRCDIR)/*.c)
OBJS=$(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRCS))
TESTSRCS=$(wildcard $(TESTDIR)/*.c)
TESTOBJS=$(patsubst $(TESTDIR)/%.c, $(TESTOBJDIR)/%.o, $(TESTSRCS))

EXEC=$(BUILDDIR)/db
TESTEXEC=$(BUILDDIR)/dbtest

all: $(EXEC)

run: all
	./$(EXEC) $(filter-out $@, $(MAKECMDGOALS))

$(EXEC): $(OBJS) | $(BUILDDIR)
	$(CC) $(CFLAGS) $^ -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

test: $(TESTEXEC)
	@$(TESTEXEC)

$(TESTEXEC): $(TESTOBJS) | $(TESTDIR)
	$(CC) $(CFLAGS) $^ -o $@

$(TESTOBJDIR)/%.o: $(TESTDIR)/%.c | $(TESTOBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILDDIR) $(OBJDIR) $(TESTOBJDIR):
	mkdir -p $@

clean-obj:
	rm -rf $(OBJDIR) $(TESTOBJDIR)

clean-test:
	rm -rf $(TESTEXEC) $(TESTOBJDIR)

clean: clean-obj clean-test
	rm $(EXEC)

%:
	@:
