# Directories
SRCDIR  := source
INCDIR  := include
BINDIR  := binary
OBJDIR  := object
LIBDIR	:= lib
TESTDIR	:= tests
DEPDIR	:= dependencies
PROTODIR := proto
PROTBUF	:= /usr/include/protobuf-c/

# Compiler and linker options
CC      := gcc
CFLAGS  := -Wall -O3 -g -I ./$(INCDIR) 
LDFLAGS	:= -I $(PROTBUF) -lprotobuf-c -lzookeeper_mt
DEPFLAGS := -MMD
AR		:= ar
ARFLAGS	:= rcs

# sources
SRC_UTILS	:= $(SRCDIR)/utils.c $(SRCDIR)/aptime.c
OBJ_UTILS	:= $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRC_UTILS))

SRC_GENERIC := $(SRCDIR)/data.c $(SRCDIR)/entry.c $(SRCDIR)/list.c $(SRCDIR)/table.c $(SRCDIR)/stats.c $(SRCDIR)/replicator.c
OBJ_GENERIC := $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRC_GENERIC))

SRC_SERVER := $(SRCDIR)/network_server.c $(SRCDIR)/table_skel.c $(SRCDIR)/database.c $(SRCDIR)/client_executor.c
OBJ_SERVER := $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRC_SERVER)) 

SRC_CLIENT := $(SRCDIR)/client_stub.c $(SRCDIR)/network_client.c 
OBJ_CLIENT := $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRC_CLIENT))

PROTO_FILES = $(wildcard $(PROTODIR)/*.proto)
SRC_PROTO := $(patsubst $(PROTODIR)/%.proto, $(SRCDIR)/%.pb-c.c, $(PROTO_FILES))
OBJ_PROTO := $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRC_PROTO))

SRC_MSG := $(SRCDIR)/sdmessage.pb-c.c $(SRCDIR)/message.c
OBJ_MSG := $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRC_MSG))

.PHONY: all clean generate_protos libmessages libutils libtable libserver libclient table-server table-client

libmessages: $(OBJ_MSG) $(LIBDIR)/libmessages.a
libutils: $(OBJ_UTILS) $(LIBDIR)/libutils.a
libtable: libutils $(OBJ_GENERIC) $(LIBDIR)/libtable.a
libserver: libmessages libtable $(OBJ_SERVER) $(LIBDIR)/libserver.a
libclient: libmessages libtable $(OBJ_CLIENT) $(LIBDIR)/libclient.a
table-server: libserver $(BINDIR)/table-server
table-client: libclient $(BINDIR)/table-client

all: libmessages libtable table-server table-client

$(SRCDIR)/sdmessage.pb-c.c: $(PROTODIR)/sdmessage.proto
	protoc-c --proto_path=$(PROTODIR) --c_out=proto sdmessage.proto
	mv $(PROTODIR)/sdmessage.pb-c.h $(INCDIR)
	mv $(PROTODIR)/sdmessage.pb-c.c $(SRCDIR)

$(LIBDIR)/libmessages.a: $(OBJ_MSG)
	$(AR) $(ARFLAGS) $@ $^

$(LIBDIR)/libutils.a: $(OBJDIR)/utils.o $(OBJDIR)/aptime.o
	$(AR) $(ARFLAGS) $@ $^

$(LIBDIR)/libtable.a: $(OBJ_GENERIC) 
	$(AR) $(ARFLAGS) $@ $^ $(LIBDIR)/libutils.a

$(LIBDIR)/libserver.a: $(OBJ_SERVER)
	$(AR) $(ARFLAGS) $@ $^ $(LIBDIR)/libtable.a $(LIBDIR)/libutils.a $(LIBDIR)/libmessages.a

$(LIBDIR)/libclient.a: $(OBJ_CLIENT)
	$(AR) $(ARFLAGS) $@ $^ $(LIBDIR)/libtable.a $(LIBDIR)/libutils.a $(LIBDIR)/libmessages.a

$(BINDIR)/table-server: $(OBJDIR)/table_server.o $(LIBDIR)/libserver.a
	$(CC) $< -o $@ -L$(LIBDIR) -lserver -ltable -lutils -lmessages $(LDFLAGS)

$(BINDIR)/table-client: $(OBJDIR)/table_client.o $(LIBDIR)/libclient.a
	$(CC) $(CFLAGS) $< -o $@ -L$(LIBDIR) -lclient -ltable -lutils -lmessages $(LDFLAGS)

$(TESTDIR)/test_%: $(OBJDIR)/test_%.o $(LIBDIR)/libtable.a
	$(CC) $< -o $@ -L$(LIBDIR) -ltable $(LDFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) $(DEPFLAGS) -MF $(DEPDIR)/$*.d -c $< -o $@

clean:
	rm -rf $(BINDIR)/*
	rm -rf $(OBJDIR)/*
	rm -rf $(LIBDIR)/*
	rm -rf $(TESTDIR)/*
	rm -rf $(DEPDIR)/*
	rm -rf $(SRCDIR)/*.pb-c.c
	rm -rf $(INCDIR)/*.pb-c.h


# include all the dependency files that have been generated
include $(wildcard $(DEPDIR)/*.d)