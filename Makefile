# Directories
SRCDIR  := source
INCDIR  := include
BINDIR  := binary
OBJDIR  := object

# Compiler and linker options
CC      := gcc
CFLAGS  := -Wall -O3 -g -I ./$(INCDIR)
LDFLAGS	:=


# Source files (all)
SRCS    := $(wildcard $(SRCDIR)/*.c)
OBJS    := $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRCS))

# Source files excluding those starting with "test_"
SRCS_NO_TEST := $(filter-out $(SRCDIR)/test_%.c, $(SRCS))
OBJS_NO_TEST := $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRCS_NO_TEST))

# Target binaries of tests
TEST_TARGETS := $(patsubst $(SRCDIR)/test_%.c, $(BINDIR)/test_%, $(SRCS))

# Rules
.PHONY: all clean

all: $(OBJS) $(TEST_TARGETS)

$(BINDIR)/test_%: $(OBJDIR)/test_%.o $(OBJS_NO_TEST)
	$(CC) $< $(OBJS_NO_TEST) -o $@ $(LDFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BINDIR)/*
	rm -rf $(OBJDIR)/*