#include "entry.h"
#include "entry-private.h"
#include "utils.h"

#include <stdlib.h>
#include <string.h>

struct entry_t *entry_create(char *key, struct data_t *data) {
    struct entry_t* entry = create_dynamic_memory(sizeof(struct entry_t));
    if (assert_error(
        entry == NULL,
        "entry_create",
        ERROR_MALLOC
    )) return NULL;

    entry->key = key;
    entry->value = data;
    return entry;
}

enum MemoryOperationStatus entry_cleanup(struct entry_t* entry) {
    if (assert_error(
        entry == NULL,
        "entry_cleanup",
        ERROR_DESTROY_ENTRY
    )) return M_ERROR;

    destroy_dynamic_memory(entry->key);
    if (assert_error(
        data_destroy(entry->value) == M_ERROR,
        "entry_cleanup",
        ERROR_DESTROY_ENTRY
    )) return M_ERROR;

    entry->key = NULL;
    entry->value = NULL;
    return M_OK;
}

int entry_destroy(struct entry_t *entry) {
    if (entry_cleanup(entry) == M_ERROR)
        return M_ERROR;
    destroy_dynamic_memory(entry);
    return M_OK;
}

struct entry_t *entry_dup(struct entry_t *entry) {
    char* key_copy = duplicate_memory(entry->key, strlen(entry->key), "entry_dup");
    if (key_copy == NULL)
        return NULL;

    void* data_copy = data_dup(entry->value);
    if (data_copy == NULL) {
        destroy_dynamic_memory(key_copy);
        return NULL;
    }
        
    struct entry_t* duplicate = entry_create(key_copy, data_copy);
    if (assert_error(
        duplicate == NULL,
        "entry_dup",
        ERROR_CREATE_ENTRY
    )) {
        destroy_dynamic_memory(key_copy);
        destroy_dynamic_memory(data_copy);
        return NULL;
    }

    return duplicate; 
}

int entry_replace(struct entry_t *entry, char *new_key, struct data_t *new_value) {
    if (entry_cleanup(entry) == M_ERROR)
        return M_ERROR;

    entry->key = new_key;
    entry->value = new_value;
    return M_OK;
}

int entry_compare(struct entry_t *entry1, struct entry_t *entry2) {
    return (entry1 == NULL || entry2 == NULL) ? CMP_ERROR : string_compare(entry1->key, entry2->key);
}