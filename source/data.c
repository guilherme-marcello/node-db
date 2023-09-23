#include "data.h"
#include "data-private.h"
#include "main-private.h"

#include <stdlib.h>
#include <string.h>


void safe_free(void* ptr) {
    if (ptr) free(ptr);
}

void* create_dynamic_memory(int size) {
    return calloc(1, size);
}

void destroy_dynamic_memory(void* ptr) {
    safe_free(ptr);
}

struct data_t *data_create(int size, void *data) {
    struct data_t* block = create_dynamic_memory(sizeof(struct data_t));
    if (assert_error(
        block == NULL,
        "data_create",
        ERROR_MALLOC
    )) return NULL;
    
    block->data = data;
    block->datasize = size;
    return block;
}

int data_destroy(struct data_t *data) {
    if (assert_error(
        data == NULL,
        "data_destroy",
        ERROR_DESTROY_DATA
    )) return ERROR;

    destroy_dynamic_memory(data->data);
    destroy_dynamic_memory(data);
    return OK;
}


struct data_t *data_dup(struct data_t *data) {
    void* data_copy = create_dynamic_memory(data->datasize);
    if (assert_error(
        data_copy == NULL,
        "data_dup",
        ERROR_MALLOC
    )) return NULL;

    if (assert_error(
        memcpy(data_copy, data->data, data->datasize) == NULL,
        "data_dup",
        ERROR_MEMCPY
    )) {
        destroy_dynamic_memory(data_copy);
        return NULL;
    }

    struct data_t* duplicate = data_create(data->datasize, data_copy);
    if (assert_error(
        duplicate == NULL,
        "data_dup",
        ERROR_CREATE_DATA
    )) {
        destroy_dynamic_memory(data_copy);
        return NULL;
    }

    return duplicate;
}


int data_replace(struct data_t *data, int new_size, void *new_data) {
    if (assert_error(
        data == NULL,
        "data_replace",
        ERROR_DESTROY_DATA
    )) return ERROR;

    destroy_dynamic_memory(data->data);
    
    data->data = new_data;
    data->datasize = new_size;
    return OK;
}