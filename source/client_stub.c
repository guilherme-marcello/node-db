#include "client_stub.h"
#include "network_client.h"
#include "client_stub-private.h"

#include "entry.h"
#include "utils.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>


struct rtable_t* rtable_create(char* address_port) {
    // split 
    char *ip_str = strtok(address_port, ":");
    char *port_str = strtok(NULL, ":");
    if (assert_error(
        ip_str == NULL || port_str == NULL,
        "rtable_create",
        "Invalid connection string format. It should be address:port.\n"
    )) return NULL;

    struct rtable_t* table = create_dynamic_memory(sizeof(struct rtable_t));
    if (assert_error(
        table == NULL,
        "rtable_create",
        ERROR_MALLOC
    )) return NULL;

    table->server_address = ip_str;
    table->server_port = atoi(port_str);
    table->sockfd = -1;
    
    return table;
}

int rtable_destroy(struct rtable_t *rtable) {
    if (assert_error(
        rtable == NULL,
        "rtable_destroy",
        ERROR_NULL_POINTER_REFERENCE
    )) return M_ERROR;

    close(rtable->sockfd);
    safe_free(rtable);
    return M_OK;
}

struct rtable_t *rtable_connect(char *address_port) {
    if (assert_error(
        address_port == NULL,
        "rtable_connect",
        ERROR_NULL_POINTER_REFERENCE   
    )) return NULL;

    struct rtable_t* table = rtable_create(address_port);
    if (assert_error(
        table == NULL,
        "rtable_connect",
        "Failed to connect to remote table.\n"
    )) return NULL;

    if (assert_error(
        network_connect(table) == -1,
        "rtable_connect",
        "Failed to connect to the table server.\n"
    )) {
        rtable_destroy(table);
        return NULL;
    }
    return table;
}

int rtable_disconnect(struct rtable_t *rtable) {
    return rtable_destroy(rtable);
}

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

    msg_wrapper->opcode = MESSAGE_T__OPCODE__OP_PUT;
    msg_wrapper->c_type = MESSAGE_T__C_TYPE__CT_ENTRY;
    return msg_wrapper;
}

bool was_operation_successful(MessageT* sent, MessageT* received) {
    if (sent == NULL || received == NULL)
        return false;

    return received->c_type == MESSAGE_T__C_TYPE__CT_NONE
        && received->opcode == sent->opcode + 1;
}

int rtable_put(struct rtable_t *rtable, struct entry_t *entry) {
    if (assert_error(
        rtable == NULL || entry == NULL,
        "rtable_put",
        ERROR_NULL_POINTER_REFERENCE
    )) return -1;

    // create entry
    EntryT* entry_wrapper = wrap_entry(entry);
    if (entry_wrapper == NULL)
        return -1;

    MessageT* msg_wrapper = wrap_message(MESSAGE_T__OPCODE__OP_PUT, MESSAGE_T__C_TYPE__CT_ENTRY);
    if (msg_wrapper == NULL) {
        entry_t__free_unpacked(entry_wrapper, NULL);
        return -1;
    }

    msg_wrapper->entry = entry_wrapper;

    // send a wait for response...
    MessageT* received = network_send_receive(rtable, msg_wrapper);
    if (!was_operation_successful(msg_wrapper, received)) {
        entry_t__free_unpacked(entry_wrapper, NULL);
        message_t__free_unpacked(msg_wrapper, NULL);
        if (received != NULL)
            message_t__free_unpacked(received, NULL);
        return -1;
    }
    
    return 0;
}

struct data_t* unwrap_data_from_message(MessageT* msg) {
    if (msg == NULL || msg->value.data == NULL)
        return NULL;

    // copy data field from message...
    void* value = duplicate_memory(msg->value.data, msg->value.len, "unwrap_data");
    if (value == NULL)
        return NULL;

    // wrap in data_t..
    struct data_t* data = data_create(msg->value.len, value);
    if (data == NULL) {
        destroy_dynamic_memory(value);
        return NULL;
    }
    return data;
}

struct data_t *rtable_get(struct rtable_t *rtable, char *key) {
    return NULL;
}

int rtable_del(struct rtable_t *rtable, char *key) {
    return 0;
}

int rtable_size(struct rtable_t *rtable) {
    return 0;
}

char **rtable_get_keys(struct rtable_t *rtable) {
    return NULL;
}

void rtable_free_keys(char **keys) {
    return;
}

struct entry_t **rtable_get_table(struct rtable_t *rtable) {
    return NULL;
}

void rtable_free_entries(struct entry_t **entries) {
    return;
}