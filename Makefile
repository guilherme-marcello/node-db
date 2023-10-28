# Directories
SRCDIR  := source
INCDIR  := include
BINDIR  := binary
OBJDIR  := object
LIBDIR	:= lib
TESTDIR	:= tests

# Compiler and linker options
CC      := gcc
CFLAGS  := -Wall -O3 -g -I ./$(INCDIR)
LDFLAGS	:=
AR		:= ar
ARFLAGS	:= rcs

# Source files (all)
SRCS    := $(wildcard $(SRCDIR)/*.c)
OBJS    := $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRCS))

# Source files excluding those starting with "test_"
SRCS_NO_TEST := $(filter-out $(SRCDIR)/test_%.c, $(SRCS))
# Source files excluding those starting with "table_" and "test_"
SRCS_NO_MAIN := $(filter-out $(SRCDIR)/table_%.c, $(SRCS_NO_TEST))

OBJS_NO_MAIN := $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRCS_NO_MAIN))

# Target binaries of tests
TEST_TARGETS := $(patsubst $(SRCDIR)/test_%.c, $(TESTDIR)/test_%, $(SRCS))

# Target lib
LIB_TARGET := $(LIBDIR)/libtable.a

# Target binaries of server/client
MAIN_TARGET_CLIENT := $(BINDIR)/table-client
MAIN_TARGET_SERVER := $(BINDIR)/table-server


# Rules
.PHONY: all clean libtable table-client table-server tests

libtable: $(OBJS) $(LIB_TARGET)
tests: libtable $(TEST_TARGETS)
table-client: libtable $(MAIN_TARGET_CLIENT)
table-server: libtable $(MAIN_TARGET_SERVER)

$(LIBDIR)/libtable.a: $(OBJS_NO_MAIN)
	$(AR) $(ARFLAGS) $@ $^


all: libtable table-client table-server tests

$(BINDIR)/table-%: $(OBJDIR)/table_%.o $(LIBDIR)/libtable.a
	$(CC) $< -o $@ -L$(LIBDIR) -ltable $(LDFLAGS)

$(TESTDIR)/test_%: $(OBJDIR)/test_%.o $(LIBDIR)/libtable.a
	$(CC) $< -o $@ -L$(LIBDIR) -ltable $(LDFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BINDIR)/*
	rm -rf $(OBJDIR)/*
	rm -rf $(LIBDIR)/*