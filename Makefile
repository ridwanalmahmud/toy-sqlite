CC=clang
INCDIR=include
SRCDIR=src
BUILDDIR=build
OBJDIR=$(BUILDDIR)/obj
TESTDIR=tests
TESTBUILDDIR=$(TESTDIR)/build
TESTOBJDIR=$(TESTBUILDDIR)/obj
OPT=-O2
CFLAGS=-Wall -Wextra -I$(INCDIR) -pipe -pedantic -D_FORTIFY_SOURCE=2 -D_GNU_SOURCE $(OPT) \
	   -fstack-protector-all -fPIE -MMD -MP \
	   -g
LDFLAGS=-pie
TESTLIB=-lgtest -lstdc++

SRCS=$(wildcard $(SRCDIR)/*.c)
OBJS=$(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRCS))
DEPS=$(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.d, $(SRCS))
EXEC=$(BUILDDIR)/db

TESTSRCS=$(wildcard $(TESTDIR)/*.cpp)
TESTOBJS=$(patsubst $(TESTDIR)/%.cpp, $(TESTOBJDIR)/%.o, $(TESTSRCS))
TESTDEPS=$(patsubst $(TESTDIR)/%.cpp, $(TESTOBJDIR)/%.d, $(TESTSRCS))
TESTEXEC=$(TESTBUILDDIR)/dbtest

-include $(DEPS)
-include $(TESTDEPS)

.PHONY: all run test clean-obj clean-test clean

all: $(EXEC)

run: $(EXEC)
	@$(EXEC)

$(EXEC): $(OBJS) | $(BUILDDIR)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

test: $(TESTEXEC)
	@$(TESTEXEC)

$(TESTEXEC): $(TESTOBJS) $(filter-out $(OBJDIR)/main.o, $(OBJS)) | $(TESTBUILDDIR)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@ $(TESTLIB)

$(TESTOBJDIR)/%.o: $(TESTDIR)/%.cpp | $(TESTOBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILDDIR) $(OBJDIR) $(TESTBUILDDIR) $(TESTOBJDIR):
	mkdir -p $@

clean-obj:
	rm -rf $(OBJDIR) $(TESTOBJDIR)

clean-test:
	rm -rf $(TESTEXEC) $(TESTOBJDIR)

clean: clean-obj clean-test
	rm $(EXEC)

# only if passing arguments are needed
%:
	@:
