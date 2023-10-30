#include "message.h"
#include "utils.h"
#include "sdmessage.pb-c.h"
#include "entry.h"

#include <stdbool.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>


EntryT* wrap_entry(struct entry_t* entry) {
    EntryT* entry_wrapper = create_dynamic_memory(sizeof(EntryT));
    if (assert_error(
        entry_wrapper == NULL,
        "wrap_entry",
        ERROR_MALLOC
    )) return NULL;
    entry_t__init(entry_wrapper);
    entry_wrapper->key = entry->key;
    entry_wrapper->value.data = entry->value->data;
    entry_wrapper->value.len = entry->value->datasize;
    return entry_wrapper;
}

MessageT* wrap_message(MessageT__Opcode opcode, MessageT__CType ctype) {
    MessageT* msg_wrapper = create_dynamic_memory(sizeof(MessageT));
    if (assert_error(
        msg_wrapper == NULL,
        "wrap_message",
        ERROR_MALLOC
    )) return NULL;
    message_t__init(msg_wrapper);

    msg_wrapper->opcode = opcode;
    msg_wrapper->c_type = ctype;
    return msg_wrapper;
}


struct data_t* unwrap_data_from_entry(EntryT* entry) {
    if (assert_error(
        entry == NULL || entry->value.data == NULL,
        "unwrap_data_from_message",
        ERROR_MALLOC
    )) return NULL;

    // copy data field from message...
    void* value = duplicate_memory(entry->value.data, entry->value.len, "unwrap_data");
    if (assert_error(
        value == NULL,
        "unwrap_data_from_message",
        "Failed to duplicate memory from message.\n"
    )) return NULL;

    // wrap in data_t..
    struct data_t* data = data_create(entry->value.len, value);
    if (assert_error(
        data == NULL,
        "unwrap_data_from_message",
        "Failed to create data_t from data extracted from message.\n"
    )) {
        destroy_dynamic_memory(value);
        return NULL;
    };

    return data;
}

struct data_t* unwrap_data_from_message(MessageT* msg) {
    if (assert_error(
        msg == NULL || msg->value.data == NULL,
        "unwrap_data_from_message",
        ERROR_MALLOC
    )) return NULL;

    // copy data field from message...
    void* value = duplicate_memory(msg->value.data, msg->value.len, "unwrap_data");
    if (assert_error(
        value == NULL,
        "unwrap_data_from_message",
        "Failed to duplicate memory from message.\n"
    )) return NULL;

    // wrap in data_t..
    struct data_t* data = data_create(msg->value.len, value);
    if (assert_error(
        data == NULL,
        "unwrap_data_from_message",
        "Failed to create data_t from data extracted from message.\n"
    )) {
        destroy_dynamic_memory(value);
        return NULL;
    };

    return data;
}

bool was_operation_unsuccessful(MessageT* received) {
    if (received == NULL)
        return true;
    
    return received->c_type == MESSAGE_T__C_TYPE__CT_NONE
        && received->opcode == MESSAGE_T__OPCODE__OP_ERROR;
}

MessageT* read_message(int fd) {
    // get request message size
    unsigned short msg_size_be;
    if (assert_error(
        read_all(fd, &msg_size_be, sizeof(msg_size_be)) != sizeof(msg_size_be),
        "read_message",
        "Failed to receive message's size.\n"
    )) return NULL;

    size_t msg_size = ntohs(msg_size_be);

    // allocate memory to receive message
    void* buffer = (uint8_t*)create_dynamic_memory(msg_size * sizeof(uint8_t*));
    if (assert_error(
        buffer == NULL,
        "network_receive",
        ERROR_MALLOC
    )) return NULL;

    // copy message to buffer..
    if (assert_error(
        read_all(fd, buffer, msg_size) != msg_size,
        "network_receive",
        "Failed to read client request.\n"
    )) {
        destroy_dynamic_memory(buffer);
        return NULL;
    }

    // unpack message
    MessageT *msg_request = message_t__unpack(NULL, msg_size, buffer);
    destroy_dynamic_memory(buffer);

    return msg_request;
}

int send_message(int fd, MessageT *msg) {
    if (assert_error(
        msg == NULL,
        "network_send_message",
        ERROR_NULL_POINTER_REFERENCE
    )) return -1;

    size_t msg_size = message_t__get_packed_size(msg);
    unsigned short msg_size_be = htons(msg_size); // reorder bytes to be

    // allocate buffer with message size
    uint8_t* buffer = (uint8_t*)create_dynamic_memory(msg_size * sizeof(uint8_t*));
    if (assert_error(
        buffer == NULL,
        "network_send",
        ERROR_MALLOC
    )) return -1;

    message_t__pack(msg, buffer);

    // send size
    if (assert_error(
        write_all(fd, &msg_size_be, sizeof(msg_size_be)) != sizeof(msg_size_be),
        "network_send_message",
        "Failed to send size of message.\n"
    )) {
        destroy_dynamic_memory(buffer);
        return -1;
    }

    // send msg buffer
    if (assert_error(
        write_all(fd, (void*)buffer, msg_size) != msg_size,
        "network_send_message",
        "Failed to send message buffer.\n"
    )) {
        destroy_dynamic_memory(buffer);
        return -1;
    }

    destroy_dynamic_memory(buffer);
    return 0;
}

ssize_t write_all(int sock, const void *buf, size_t n) {
    size_t bytes_written = 0;
    size_t bytes_to_write;
    const void *write_pointer;

    while (bytes_written < n) {
        bytes_to_write = n - bytes_written;
        write_pointer = buf + bytes_written;

        ssize_t res = write(sock, write_pointer, bytes_to_write);
        if (res < 0) {
            if (errno == EINTR)
                continue;
            else {
                assert_error(
                    1,
                    "write_n_to_sock",
                    "Failed to write to the socket."
                );
                return -1;
            }
        }

        bytes_written += res;
    }

    return bytes_written;
}

ssize_t read_all(int sock, void *buf, size_t n) {
    size_t bytes_read = 0;
    size_t bytes_to_read;
    void* read_pointer;

    while (bytes_read < n) {
        bytes_to_read = n - bytes_read;
        read_pointer = buf + bytes_read;

        ssize_t res = read(sock, read_pointer, bytes_to_read);
        if (res < 0) {
            if (errno == EINTR) 
                continue;
            else {
                assert_error(
                    1,
                    "read_n_from_sock",
                    "Failed to read from the socket."
                );
                return -1;
            }
        }
        
        if (res == 0)
            break;

        bytes_read += res;
    }

    return bytes_read;
}