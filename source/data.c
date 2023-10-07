#include "data.h"
#include "data-private.h"
#include "utils.h"

#include <stdlib.h>
#include <string.h>

struct data_t *data_create(int size, void *data) {
    if (assert_error(
        data == NULL,
        "data_create",
        ERROR_NULL_POINTER_REFERENCE
    )) return NULL;

    if (assert_error(
        size <= 0,
        "data_create",
        ERROR_SIZE
    )) return NULL;

    // allocate memory to the data_t block
    struct data_t* block = create_dynamic_memory(sizeof(struct data_t));
    if (assert_error(
        block == NULL,
        "data_create",
        ERROR_MALLOC
    )) return NULL;

    // update struct fields with given pointers
    block->data = data;
    block->datasize = size;
    return block;
}

enum MemoryOperationStatus data_cleanup(struct data_t *data) {
    if (assert_error(
        data == NULL,
        "data_cleanup",
        ERROR_NULL_POINTER_REFERENCE
    )) return M_ERROR;

    // destroy memory allocated by data->data and set to NULL
    destroy_dynamic_memory(data->data);
    data->data = NULL;  
    return M_OK;
}

int data_destroy(struct data_t *data) {
    // clean up data
    if (data_cleanup(data) == M_ERROR)
        return M_ERROR;

    // destroy data_t struct
    destroy_dynamic_memory(data);
    return M_OK;
}

struct data_t *data_dup(struct data_t *data) {
    if (assert_error(
        data == NULL,
        "data_dup",
        ERROR_NULL_POINTER_REFERENCE
    )) return NULL;

    // try to duplicate data block
    void* data_copy = duplicate_memory(data->data, data->datasize, "data_dup");
    if (data_copy == NULL)
        return NULL;

    // create data with new copied data
    struct data_t* duplicate = data_create(data->datasize, data_copy);
    if (assert_error(
        duplicate == NULL,
        "data_dup",
        ERROR_CREATE_DATA
    )) {
        // destroy copy in case of error
        destroy_dynamic_memory(data_copy);
        return NULL;
    }

    // return duplicate
    return duplicate;
}

int data_replace(struct data_t *data, int new_size, void *new_data) {
    // clean up data
    if (data_cleanup(data) == M_ERROR)
        return M_ERROR;

    // update struct fields with given pointers
    data->data = new_data;
    data->datasize = new_size;
    return M_OK;
}