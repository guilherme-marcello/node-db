#include "table_skel.h"
#include "table_skel-private.h"
#include "table.h"
#include "message.h"
#include "utils.h"
#include "sdmessage.pb-c.h"
#include "table_server.h"

#include <stdio.h>
#include <string.h>

struct table_t *table_skel_init(int n_lists) {
    return table_create(n_lists);
}

int table_skel_destroy(struct table_t* table) {
    return table_destroy(table);
}

int invoke(MessageT* msg, struct TableServerDatabase* db) {
    if (assert_error(
        msg == NULL || db == NULL || db->table == NULL,
        "invoke",
        ERROR_NULL_POINTER_REFERENCE
    )) return -1;

    switch (msg->opcode) {
        case MESSAGE_T__OPCODE__OP_PUT:
            printf("Received %s request!\n", "put");
            return put(msg, db);        
        case MESSAGE_T__OPCODE__OP_GET:
            printf("Received %s request!\n", "get");
            return get(msg, db);
        case MESSAGE_T__OPCODE__OP_DEL:
            printf("Received %s request!\n", "del");
            return del(msg, db);
        case MESSAGE_T__OPCODE__OP_SIZE:
            printf("Received %s request!\n", "size");
            return size(msg, db);
        case MESSAGE_T__OPCODE__OP_GETKEYS:
            printf("Received %s request!\n", "getkeys");
            return getkeys(msg, db);
        case MESSAGE_T__OPCODE__OP_GETTABLE:
            printf("Received %s request!\n", "gettable");
            return gettable(msg, db);
        default:
            printf("Received unknown request...ignoring!\n");
            return error(msg);
    }
    return 0;
}

int error(MessageT* msg) {
    if (assert_error(
        msg == NULL,
        "invoke",
        ERROR_NULL_POINTER_REFERENCE
    )) return -1;

    msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
    msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
    return 0;
}

int put(MessageT* msg, struct TableServerDatabase* db) {

    if (assert_error(
        msg == NULL || db == NULL || db->table == NULL || msg->entry == NULL ||
        msg->entry->key == NULL || msg->entry->value.data == NULL,
        "invoke",
        ERROR_NULL_POINTER_REFERENCE
    )) return -1;

    struct table_t* table = db->table;

    if (assert_error(
        msg->entry->value.len < 0,
        "invoke",
        ERROR_SIZE
    )) return -1;

    if (assert_error(
        msg->c_type != MESSAGE_T__C_TYPE__CT_ENTRY,
        "invoke",
        "Invalid c_type.\n"
    )) return -1;

    // unwrap data
    struct data_t* data = unwrap_data_from_entry(msg->entry);
    if (assert_error(
        data == NULL,
        "invoke_put",
        "Failed to unwrap data from message.\n"
    )) return error(msg);

    // put
    if (assert_error(
        table_put(table, msg->entry->key, data) == -1,
        "invoke",
        "Failed to put entry.\n"
    )) {
        data_destroy(data);
        return error(msg);
    }

    // destroy data (since it's copied during put...)
    data_destroy(data);

    msg->opcode = MESSAGE_T__OPCODE__OP_PUT + 1;
    msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
    return 0;
}

int get(MessageT* msg, struct TableServerDatabase* db) {
    if (assert_error(
        msg == NULL || db == NULL || db->table == NULL || msg->key == NULL,
        "invoke",
        ERROR_NULL_POINTER_REFERENCE
    )) return -1;

    struct table_t* table = db->table;

    if (assert_error(
        msg->c_type != MESSAGE_T__C_TYPE__CT_KEY,
        "invoke",
        "Invalid c_type.\n"
    )) return -1;

    struct data_t *data = table_get(table, msg->key);
    if (data == NULL)
        return error(msg);

    
    msg->value.len = data->datasize;
    msg->value.data = data->data;
    // destroy only the pointer
    destroy_dynamic_memory(data);
    msg->opcode = MESSAGE_T__OPCODE__OP_GET + 1;
    msg->c_type = MESSAGE_T__C_TYPE__CT_VALUE;
    return 0;
}

int del(MessageT* msg, struct TableServerDatabase* db) {
    if (assert_error(
        msg == NULL || db == NULL || db->table == NULL || msg->key == NULL,
        "invoke",
        ERROR_NULL_POINTER_REFERENCE
    )) return -1;

    struct table_t* table = db->table;

    if (assert_error(
        msg->c_type != MESSAGE_T__C_TYPE__CT_KEY,
        "invoke",
        "Invalid c_type.\n"
    )) return -1;

    if (assert_error(
        table_remove(table, msg->key) == -1,
        "invoke_get",
        "Failed to remove entry from table.\n"
    )) return error(msg);

    msg->opcode = MESSAGE_T__OPCODE__OP_DEL + 1;
    msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
    return 0;
}

int size(MessageT* msg, struct TableServerDatabase* db) {
    if (assert_error(
        msg == NULL || db == NULL || db->table == NULL,
        "invoke",
        ERROR_NULL_POINTER_REFERENCE
    )) return -1;

    struct table_t* table = db->table;

    if (assert_error(
        msg->c_type != MESSAGE_T__C_TYPE__CT_NONE,
        "invoke",
        "Invalid c_type.\n"
    )) return -1;

    msg->result = table_size(table);
    if (assert_error(
        msg->result < 0,
        "invoke_size",
        "Failed to retrieve table size.\n"
    )) return error(msg);

    // all good!
    msg->opcode = MESSAGE_T__OPCODE__OP_SIZE + 1;
    msg->c_type = MESSAGE_T__C_TYPE__CT_RESULT;

    return 0;
}

int getkeys(MessageT* msg, struct TableServerDatabase* db) {
    if (assert_error(
        msg == NULL || db == NULL || db->table == NULL,
        "invoke",
        ERROR_NULL_POINTER_REFERENCE
    )) return -1;

    struct table_t* table = db->table;

    if (assert_error(
        msg->c_type != MESSAGE_T__C_TYPE__CT_NONE,
        "invoke",
        "Invalid c_type.\n"
    )) return -1;

    char** keys = table_get_keys(table);
    if (assert_error(
        keys == NULL,
        "invoke_getkeys",
        "Failed to get keys from table.\n"
    )) return error(msg);

    int n_keys = table_size(table);
    if (assert_error(
        n_keys < 0,
        "invoke_getkeys",
        "Failed to retrieve table size.\n"
    )) {
        table_free_keys(keys);
        return error(msg);   
    }

    msg->n_keys = n_keys;
    msg->keys = keys;
    msg->opcode = MESSAGE_T__OPCODE__OP_GETKEYS + 1;
    msg->c_type = MESSAGE_T__C_TYPE__CT_KEYS;
    return 0;
}

int gettable(MessageT* msg, struct TableServerDatabase* db) {
    if (assert_error(
        msg == NULL || db == NULL || db->table == NULL,
        "invoke",
        ERROR_NULL_POINTER_REFERENCE
    )) return -1;
    
    struct table_t* table = db->table;

    if (assert_error(
        msg->c_type != MESSAGE_T__C_TYPE__CT_NONE,
        "invoke",
        "Invalid c_type.\n"
    )) return -1;

    char** keys = table_get_keys(table);
    if (assert_error(
        keys == NULL,
        "invoke",
        ERROR_MALLOC
    )) return error(msg);

    int n_entries = table_size(table);
    if (assert_error(
        n_entries < 0,
        "invoke",
        ERROR_MALLOC
    )) {
        table_free_keys(keys);
        return error(msg);
    }

    EntryT** entries = create_dynamic_memory(sizeof(EntryT*) * (n_entries + 1));
    entries[n_entries] = NULL; // in case client is expecting NULL terminator

    // with keys and n_keys, iterate over table, setting new entries
    for (int i = 0; i < n_entries; i++) {
        struct data_t* data = table_get(table, keys[i]);
        if (data == NULL) {
            // destroy copied entries until now...
            for (int j = 0; j < i; j++)
                destroy_dynamic_memory(entries[j]);
            destroy_dynamic_memory(entries);
            table_free_keys(keys);
            return error(msg);
        }

        // got data for this key! wrap it into a EntryT
        entries[i] = wrap_entry_with_data(strdup(keys[i]), data);
        if (entries[i] == NULL) {
            // destroy copied entries until now...
            for (int j = 0; j < i; j++)
                destroy_dynamic_memory(entries[j]);
            destroy_dynamic_memory(entries);
            table_free_keys(keys);
            return error(msg);     
        }

        // destroy only pointer to data struct...
        destroy_dynamic_memory(data);
    }
    table_free_keys(keys);

    msg->n_entries = n_entries;
    msg->entries = entries;
    msg->opcode = MESSAGE_T__OPCODE__OP_GETTABLE + 1;
    msg->c_type = MESSAGE_T__C_TYPE__CT_TABLE;
    return 0;
}