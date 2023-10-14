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
# Source files excluding those starting with "table_" and "test_"
SRCS_NO_MAIN := $(filter-out $(SRCDIR)/table_%.c, $(SRCS_NO_TEST))

OBJS_NO_MAIN := $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRCS_NO_MAIN))

# Target binaries of tests
TEST_TARGETS := $(patsubst $(SRCDIR)/test_%.c, $(BINDIR)/test_%, $(SRCS))
# Target binaries of server/client
MAIN_TARGETS := $(patsubst $(SRCDIR)/table_%.c, $(BINDIR)/table_%, $(SRCS))




# Rules
.PHONY: all clean

all: $(OBJS) $(MAIN_TARGETS) $(TEST_TARGETS)

$(BINDIR)/table_%: $(OBJDIR)/table_%.o $(OBJS_NO_MAIN)
	$(CC) $< $(OBJS_NO_MAIN) -o $@ $(LDFLAGS)

$(BINDIR)/test_%: $(OBJDIR)/test_%.o $(OBJS_NO_MAIN)
	$(CC) $< $(OBJS_NO_MAIN) -o $@ $(LDFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BINDIR)/*
	rm -rf $(OBJDIR)/*