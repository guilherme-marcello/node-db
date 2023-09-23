# Program info
PROGRAM_NAME := main.out

# Directories
SRCDIR  := source
INCDIR  := include
BINDIR  := binary
OBJDIR  := object

# Compiler and linker options
CC      := gcc
CFLAGS  := -Wall -O3 -g -I ./$(INCDIR)
LDFLAGS=-lrt -lpthread


# Source files
SRCS    := $(wildcard $(SRCDIR)/*.c)
OBJS    := $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRCS))

# Binary target
TARGET  := $(BINDIR)/$(PROGRAM_NAME)

# Rules
.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) 

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf $(BINDIR)/*
	rm -rf $(OBJDIR)/*