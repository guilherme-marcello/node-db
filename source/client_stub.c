#include "client_stub.h"
#include "network_client.h"
#include "client_stub-private.h"

#include "entry.h"
#include "utils.h"

#include <stdio.h>
#include <stdbool.h>
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

    msg_wrapper->opcode = opcode;
    msg_wrapper->c_type = ctype;
    return msg_wrapper;
}

bool was_operation_unsuccessful(MessageT* received) {
    if (received == NULL)
        return true;
    
    return received->c_type == MESSAGE_T__C_TYPE__CT_NONE
        && received->opcode == MESSAGE_T__OPCODE__OP_ERROR;
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
    if (was_operation_unsuccessful(received)) {
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
    if (assert_error(
        rtable == NULL || key == NULL,
        "rtable_get",
        ERROR_NULL_POINTER_REFERENCE
    )) return NULL;

    MessageT* msg_wrapper = wrap_message(MESSAGE_T__OPCODE__OP_GET, MESSAGE_T__C_TYPE__CT_KEY);
    if (msg_wrapper == NULL)
        return NULL;

    msg_wrapper->key = strdup(key);

    // send a wait for response...
    MessageT* received = network_send_receive(rtable, msg_wrapper);
    if (was_operation_unsuccessful(received)) {
        message_t__free_unpacked(msg_wrapper, NULL);
        if (received != NULL)
            message_t__free_unpacked(received, NULL);
        return NULL;
    }

    struct data_t* data = unwrap_data_from_message(received);
    message_t__free_unpacked(received, NULL);

    return data;
}

int rtable_del(struct rtable_t *rtable, char *key) {
    if (assert_error(
        rtable == NULL || key == NULL,
        "rtable_get",
        ERROR_NULL_POINTER_REFERENCE
    )) return -1;

    MessageT* msg_wrapper = wrap_message(MESSAGE_T__OPCODE__OP_DEL, MESSAGE_T__C_TYPE__CT_KEY);
    if (msg_wrapper == NULL)
        return -1;
    
    msg_wrapper->key = strdup(key);

    // send a wait for response...
    MessageT* received = network_send_receive(rtable, msg_wrapper);
    if (was_operation_unsuccessful(received)) {
        message_t__free_unpacked(msg_wrapper, NULL);
        if (received != NULL)
            message_t__free_unpacked(received, NULL);
        return -1;
    }

    message_t__free_unpacked(received, NULL);
    return 0;
}

int rtable_size(struct rtable_t *rtable) {
    if (assert_error(
        rtable == NULL,
        "rtable_size",
        ERROR_NULL_POINTER_REFERENCE
    )) return -1;

    MessageT* msg_wrapper = wrap_message(MESSAGE_T__OPCODE__OP_SIZE, MESSAGE_T__C_TYPE__CT_NONE);
    if (msg_wrapper == NULL)
        return -1;
    
    // send a wait for response...
    MessageT* received = network_send_receive(rtable, msg_wrapper);
    if (was_operation_unsuccessful(received)) {
        message_t__free_unpacked(msg_wrapper, NULL);
        if (received != NULL)
            message_t__free_unpacked(received, NULL);
        return -1;
    }
    int size = received->result;
    message_t__free_unpacked(received, NULL);
    return size;
}

char **rtable_get_keys(struct rtable_t *rtable) {
    if (assert_error(
        rtable == NULL,
        "rtable_get_keys",
        ERROR_NULL_POINTER_REFERENCE
    )) return NULL;

    MessageT* msg_wrapper = wrap_message(MESSAGE_T__OPCODE__OP_GETKEYS, MESSAGE_T__C_TYPE__CT_NONE);
    if (msg_wrapper == NULL)
        return NULL;
    
    // send a wait for response...
    MessageT* received = network_send_receive(rtable, msg_wrapper);
    message_t__free_unpacked(msg_wrapper, NULL);
    if (was_operation_unsuccessful(received)) {
        if (received != NULL)
            message_t__free_unpacked(received, NULL);
        return NULL;
    }

    // allocate buffer for keys
    char** keys = create_dynamic_memory((received->n_keys + 1) * sizeof(char*));
    if (assert_error(
        keys == NULL,
        "rtable_get_keys",
        ERROR_MALLOC
    )) {
        message_t__free_unpacked(received, NULL);
        return NULL;
    }
    keys[received->n_keys] = NULL;

    // copy keys in message to keys buffer...
    for (int i = 0; i < received->n_keys; i++) {
        keys[i] = strdup(received->keys[i]);
        if (assert_error(
            keys[i] == NULL,
            "rtable_get_keys",
            ERROR_STRDUP
        )) {
            // free already duplicated keys...
            for (int j = 0; j < i; j++)
                destroy_dynamic_memory(keys[j]);
            message_t__free_unpacked(received, NULL);
            destroy_dynamic_memory(keys);
            return NULL;
        }
    }

    message_t__free_unpacked(received, NULL);
    return keys;
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