#include "entry.h"
#include "entry-private.h"
#include "utils.h"

#include <stdlib.h>
#include <string.h>

struct entry_t *entry_create(char *key, struct data_t *data) {
    if (assert_error(
        key == NULL || data == NULL,
        "entry_create",
        ERROR_NULL_POINTER_REFERENCE
    )) return NULL;

    // allocate memory for entry
    struct entry_t* entry = create_dynamic_memory(sizeof(struct entry_t));
    if (assert_error(
        entry == NULL,
        "entry_create",
        ERROR_MALLOC
    )) return NULL;

    // update struct with given pointers
    entry->key = key;
    entry->value = data;
    return entry;
}

struct entry_t* entry_copy_create(char *key, struct data_t* data) {
    if (assert_error(
        key == NULL || data == NULL,
        "entry_copy_create",
        ERROR_NULL_POINTER_REFERENCE
    )) return NULL;

    // duplicate key
    char* key_copy = strdup(key);
    if (assert_error(
        key_copy == NULL,
        "entry_copy_create",
        ERROR_STRDUP
    )) return NULL;

    // duplicate data
    struct data_t* data_copy = data_dup(data);
    if (data == NULL) {
        destroy_dynamic_memory(key_copy);
        return NULL;
    }

    // create new entry with copied key and data
    struct entry_t* entry = entry_create(key_copy, data_copy);
    if (entry == NULL) {
        // destroy allocated memory in case of error
        destroy_dynamic_memory(key_copy);
        data_destroy(data_copy);
    }

    return entry;
}

enum MemoryOperationStatus entry_cleanup(struct entry_t* entry) {
    if (assert_error(
        entry == NULL || entry->key == NULL || entry->value == NULL,
        "entry_cleanup",
        ERROR_NULL_POINTER_REFERENCE
    )) return M_ERROR;

    // destroy key
    destroy_dynamic_memory(entry->key);
    // destroy data
    if (assert_error(
        data_destroy(entry->value) == M_ERROR,
        "entry_cleanup",
        ERROR_DESTROY_ENTRY
    )) return M_ERROR;

    // set values to NULL
    entry->key = NULL;
    entry->value = NULL;
    return M_OK;
}

int entry_destroy(struct entry_t *entry) {
    // cleanup entry
    if (entry_cleanup(entry) == M_ERROR)
        return M_ERROR;

    // destroy memory of the struct itself and return M_OK
    destroy_dynamic_memory(entry);
    return M_OK;
}

struct entry_t *entry_dup(struct entry_t *entry) {
    if (assert_error(
        entry == NULL || entry->key == NULL,
        "entry_dup",
        ERROR_NULL_POINTER_REFERENCE
    )) return NULL;
    
    // duplicate key
    char* key_copy = duplicate_memory(entry->key, strlen(entry->key) + 1, "entry_dup");
    if (key_copy == NULL)
        return NULL;

    // duplicate data
    void* data_copy = data_dup(entry->value);
    if (data_copy == NULL) {
        destroy_dynamic_memory(key_copy);
        return NULL;
    }

    // create new entry with copied key and data
    struct entry_t* duplicate = entry_create(key_copy, data_copy);
    if (assert_error(
        duplicate == NULL,
        "entry_dup",
        ERROR_CREATE_ENTRY
    )) {
        // destroy copies in case of error
        destroy_dynamic_memory(key_copy);
        destroy_dynamic_memory(data_copy);
        return NULL;
    }
    // return duplicate
    return duplicate; 
}

int entry_replace(struct entry_t *entry, char *new_key, struct data_t *new_value) {
    if (assert_error(
        new_key == NULL || new_value == NULL,
        "entry_replace",
        ERROR_NULL_POINTER_REFERENCE
    )) return M_ERROR;

    // cleanup entry
    if (entry_cleanup(entry) == M_ERROR)
        return M_ERROR;

    // update key and value with given new key-value
    entry->key = new_key;
    entry->value = new_value;
    return M_OK;
}

int entry_compare(struct entry_t *entry1, struct entry_t *entry2) {
    // compare entries by comparing keys
    return (entry1 == NULL || entry2 == NULL) ? CMP_ERROR : string_compare(entry1->key, entry2->key);
}