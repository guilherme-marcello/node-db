#include "table_skel.h"
#include "table.h"
#include "message.h"
#include "utils.h"
#include "sdmessage.pb-c.h"

#include <stdio.h>

struct table_t *table_skel_init(int n_lists) {
    return table_create(n_lists);
}

int table_skel_destroy(struct table_t* table) {
    return table_destroy(table);
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

int put(MessageT* msg, struct table_t* table) {
    if (assert_error(
        msg == NULL || table == NULL || msg->entry == NULL ||
        msg->entry->key == NULL || msg->entry->value.data == NULL,
        "invoke",
        ERROR_NULL_POINTER_REFERENCE
    )) return -1;

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

int get(MessageT* msg, struct table_t* table) {
    if (assert_error(
        msg == NULL || table == NULL || msg->key == NULL,
        "invoke",
        ERROR_NULL_POINTER_REFERENCE
    )) return -1;

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

int del(MessageT* msg, struct table_t* table) {
    if (assert_error(
        msg == NULL || table == NULL,
        "invoke",
        ERROR_NULL_POINTER_REFERENCE
    )) return -1;
    return 0;
}

int size(MessageT* msg, struct table_t* table) {
    if (assert_error(
        msg == NULL || table == NULL,
        "invoke",
        ERROR_NULL_POINTER_REFERENCE
    )) return -1;
    return 0;
}

int getkeys(MessageT* msg, struct table_t* table) {
    if (assert_error(
        msg == NULL || table == NULL,
        "invoke",
        ERROR_NULL_POINTER_REFERENCE
    )) return -1;
    return 0;
}

int gettable(MessageT* msg, struct table_t* table) {
    if (assert_error(
        msg == NULL || table == NULL,
        "invoke",
        ERROR_NULL_POINTER_REFERENCE
    )) return -1;
    return 0;
}


int invoke(MessageT* msg, struct table_t* table) {
    printf("maing invoke...\n");
    if (assert_error(
        msg == NULL || table == NULL,
        "invoke",
        ERROR_NULL_POINTER_REFERENCE
    )) return -1;
    printf("maing invoke swithc......\n");
    switch (msg->opcode) {
        case MESSAGE_T__OPCODE__OP_PUT:
            printf("Received %s request!\n", "put");
            return put(msg, table);        
        case MESSAGE_T__OPCODE__OP_GET:
            printf("Received %s request!\n", "get");
            return get(msg, table);
        case MESSAGE_T__OPCODE__OP_DEL:
            printf("Received %s request!\n", "del");
            return del(msg, table);
        case MESSAGE_T__OPCODE__OP_SIZE:
            printf("Received %s request!\n", "size");
            return size(msg, table);
        case MESSAGE_T__OPCODE__OP_GETKEYS:
            printf("Received %s request!\n", "getkeys");
            return getkeys(msg, table);
        case MESSAGE_T__OPCODE__OP_GETTABLE:
            printf("Received %s request!\n", "gettable");
            return gettable(msg, table);
        default:
            printf("Received unknown request...ignoring!\n");
            return error(msg);
    }
    return 0;
}