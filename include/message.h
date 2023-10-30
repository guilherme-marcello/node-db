#ifndef _MESSAGE_H
#define _MESSAGE_H 

#include "entry.h"
#include "data.h"
#include "sdmessage.pb-c.h"

#include <unistd.h>
#include <stdbool.h>

EntryT* wrap_entry(struct entry_t* entry);
MessageT* wrap_message(MessageT__Opcode opcode, MessageT__CType ctype);
struct data_t* unwrap_data_from_message(MessageT* msg);
struct data_t* unwrap_data_from_entry(EntryT* entry);

bool was_operation_unsuccessful(MessageT* received);

int send_message(int fd, MessageT *msg);
MessageT* read_message(int fd);

ssize_t write_all(int sock, const void *buf, size_t n);
ssize_t read_all(int sock, void *buf, size_t n);

#endif