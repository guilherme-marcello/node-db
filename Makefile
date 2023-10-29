# Directories
SRCDIR  := source
INCDIR  := include
BINDIR  := binary
OBJDIR  := object
LIBDIR	:= lib
TESTDIR	:= tests
PROTBUF	:= /usr/include/protobuf-c/

# Compiler and linker options
CC      := gcc
CFLAGS  := -Wall -O3 -g -I ./$(INCDIR) -I $(PROTBUF) -lprotobuf-c
LDFLAGS	:=
AR		:= ar
ARFLAGS	:= rcs

SRC_GENERIC := $(SRCDIR)/data.c $(SRCDIR)/entry.c $(SRCDIR)/list.c $(SRCDIR)/table.c
OBJ_GENERIC := $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRC_GENERIC))

SRC_SERVER := $(SRCDIR)/network_server.c $(SRCDIR)/table_skel.c 
OBJ_SERVER := $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRC_SERVER))

.PHONY: all clean libutils libtable table-server

libutils: $(OBJDIR)/utils.o $(LIBDIR)/libutils.a
libtable: libutils $(OBJ_GENERIC) $(LIBDIR)/libtable.a
libserver: libtable $(OBJ_SERVER) $(LIBDIR)/libserver.a
table-server: libserver $(BINDIR)/table-server

$(LIBDIR)/libutils.a: $(OBJDIR)/utils.o
	$(AR) $(ARFLAGS) $@ $^

$(LIBDIR)/libtable.a: $(OBJ_GENERIC) 
	$(AR) $(ARFLAGS) $@ $^ $(LIBDIR)/libutils.a

$(LIBDIR)/libserver.a: $(OBJ_SERVER)
	$(AR) $(ARFLAGS) $@ $^ $(LIBDIR)/libtable.a $(LIBDIR)/libutils.a


all: table-server


$(BINDIR)/table-server: $(OBJDIR)/table_server.o $(LIBDIR)/libserver.a
	$(CC) $< -o $@ -L$(LIBDIR) -lserver -ltable -lutils  $(LDFLAGS)

$(TESTDIR)/test_%: $(OBJDIR)/test_%.o $(LIBDIR)/libtable.a
	$(CC) $< -o $@ -L$(LIBDIR) -ltable $(LDFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BINDIR)/*
	rm -rf $(OBJDIR)/*
	rm -rf $(LIBDIR)/*
	rm -rf $(TESTDIR)/*