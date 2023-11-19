#ifndef _MESSAGE_H
#define _MESSAGE_H 

#include "entry.h"
#include "data.h"
#include "stats.h"
#include "sdmessage.pb-c.h"

#include <unistd.h>
#include <stdbool.h>

/**
 * Wrap an ServerStatsT structure with the provided data and return a new ServerStatsT.
 *
 * @param active_clients - Amount of active clients
 * @param op_counter - Amount of operations
 * @param computed_time - Computed time in micro seconds
 * @return A new ServerStatsT structure.
 */
ServerStatsT* wrap_stats_with_data(int active_clients, int op_counter, int computed_time);

/**
 * Wrap an existing statistics_t structure into a new ServerStatsT structure.
 *
 * @param stats - The statistics_t structure to wrap.
 * @return A new ServerStatsT structure containing the same data.
 */
ServerStatsT* wrap_stats(struct statistics_t* stats);

/**
 * Wrap an EntryT structure with the provided data and return a new EntryT.
 *
 * @param key - The key to associate with the entry.
 * @param data - The data to associate with the entry.
 * @return A new EntryT structure.
 */
EntryT* wrap_entry_with_data(char* key, struct data_t* data);

/**
 * Wrap an existing entry_t structure into a new EntryT structure.
 *
 * @param entry - The entry_t structure to wrap.
 * @return A new EntryT structure containing the same data.
 */
EntryT* wrap_entry(struct entry_t* entry);

/**
 * Create a new MessageT structure with the specified opcode and content type.
 *
 * @param opcode - The operation code of the message.
 * @param ctype - The content type of the message.
 * @return A new MessageT structure.
 */
MessageT* wrap_message(MessageT__Opcode opcode, MessageT__CType ctype);

/**
 * Unwrap the data field from a MessageT structure and return a pointer to it.
 *
 * @param msg - The MessageT structure containing the data.
 * @return A pointer to the data.
 */
struct data_t* unwrap_data_from_message(MessageT* msg);

/**
 * Unwrap the data field from an EntryT structure and return a pointer to it.
 *
 * @param entry - The EntryT structure containing the data.
 * @return A pointer to the data.
 */
struct data_t* unwrap_data_from_entry(EntryT* entry);

/**
 * Check if the received message indicates an unsuccessful operation.
 *
 * @param received - The received MessageT structure to check.
 * @return True if the operation was unsuccessful, false otherwise.
 */
bool was_operation_unsuccessful(MessageT* received);

/**
 * Send a MessageT structure over the specified file descriptor (socket).
 *
 * @param fd - The file descriptor to send the message to.
 * @param msg - The MessageT structure to send.
 * @return The number of bytes sent or -1 in case of an error.
 */
int send_message(int fd, MessageT *msg);

/**
 * Read a MessageT structure from the specified file descriptor (socket).
 *
 * @param fd - The file descriptor to read the message from.
 * @return A pointer to the received MessageT structure or NULL in case of an error.
 */
MessageT* read_message(int fd);

/**
 * Write a specified number of bytes from a buffer to a socket.
 *
 * @param sock - The socket file descriptor.
 * @param buf - The buffer containing the data to write.
 * @param n - The number of bytes to write.
 * @return The number of bytes written or -1 in case of an error.
 */
ssize_t write_all(int sock, const void *buf, size_t n);

/**
 * Read a specified number of bytes from a socket into a buffer.
 *
 * @param sock - The socket file descriptor.
 * @param buf - The buffer to store the read data.
 * @param n - The number of bytes to read.
 * @return The number of bytes read or -1 in case of an error.
 */
ssize_t read_all(int sock, void *buf, size_t n);

#endif