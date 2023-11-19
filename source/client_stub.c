#include "client_stub.h"
#include "network_client.h"
#include "message.h"
#include "client_stub-private.h"

#include "entry.h"
#include "utils.h"
#include "stats.h"

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

struct entry_t **rtable_get_table(struct rtable_t *rtable) {
    if (assert_error(
        rtable == NULL,
        "rtable_get_table",
        ERROR_NULL_POINTER_REFERENCE
    )) return NULL;

    MessageT* msg_wrapper = wrap_message(MESSAGE_T__OPCODE__OP_GETTABLE, MESSAGE_T__C_TYPE__CT_NONE);
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

    // allocate buffer for entries
    struct entry_t** entries = create_dynamic_memory((received->n_entries + 1) * sizeof(struct entry*));
    if (assert_error(
        entries == NULL,
        "rtable_get_table",
        ERROR_MALLOC
    )) {
        message_t__free_unpacked(received, NULL);
        return NULL;
    }
    entries[received->n_keys] = NULL;

    // iterate over wrapped entries (from message)
    for (int i = 0; i < received->n_entries; i++) {
        void* copied_data = duplicate_memory(received->entries[i]->value.data, received->entries[i]->value.len, "rtable_get_table");
        if (assert_error(
            copied_data == NULL,
            "rtable_get_table",
            "Failed to duplicate data from message.\n"
        )) {
            // destroy already created entries..
            for (int j = 0; j < i; j++)
                entry_destroy(entries[j]);
            destroy_dynamic_memory(entries);
            message_t__free_unpacked(received, NULL);
            return NULL;
        }

        // create data_t!
        struct data_t* data = data_create(received->entries[i]->value.len, copied_data);
        if (assert_error(
            data == NULL,
            "rtable_get_table",
            "Failed to create data structure.\n"
        )) {
            destroy_dynamic_memory(copied_data);
            // destroy already created entries..
            for (int j = 0; j < i; j++)
                entry_destroy(entries[j]);
            destroy_dynamic_memory(entries);
            message_t__free_unpacked(received, NULL);
            return NULL;
        }

        // create entry!
        struct entry_t* entry = entry_create(
            strdup(received->entries[i]->key), data
        );
        if (assert_error(
            entry == NULL,
            "rtable_get_table",
            "Failed to create entry structure.\n"
        )) {
            data_destroy(data);
            // destroy already created entries..
            for (int j = 0; j < i; j++)
                entry_destroy(entries[j]);
            destroy_dynamic_memory(entries);
            message_t__free_unpacked(received, NULL);
            return NULL;
        } 

        // add to the buffer
        entries[i] = entry;
    }

    return entries;
}

void rtable_free_keys(char **keys) {
    // starting with index 0, iterate over keys, destroying
    int index = 0;
    char* key;
    while ((key = keys[index])) {
        destroy_dynamic_memory(key);
        index++;
    }
}

void rtable_free_entries(struct entry_t **entries) {
    // starting with index 0, iterate over entries, destroying them
    int index = 0;
    struct entry_t* entry;
    while ((entry = entries[index])) {
        entry_destroy(entry);
        index++;
    }
}

struct statistics_t* rtable_stats(struct rtable_t* rtable) {
    if (assert_error(
        rtable == NULL,
        "rtable_stats",
        ERROR_NULL_POINTER_REFERENCE
    )) return NULL;

    MessageT* msg_wrapper = wrap_message(MESSAGE_T__OPCODE__OP_STATS, MESSAGE_T__C_TYPE__CT_NONE);
    if (msg_wrapper == NULL)
        return NULL;
    
    // send a wait for response...
    MessageT* received = network_send_receive(rtable, msg_wrapper);
    if (was_operation_unsuccessful(received)) {
        message_t__free_unpacked(msg_wrapper, NULL);
        if (received != NULL)
            message_t__free_unpacked(received, NULL);
        return NULL;
    }
    struct statistics_t* stats = stats_create(received->stats->op_counter, received->stats->computed_time, received->stats->active_clients);
    message_t__free_unpacked(received, NULL);

    return stats;
}