#include "table.h"
#include "table-private.h"
#include "list-private.h"
#include "entry.h"
#include "entry-private.h"
#include "utils.h"

#include <stdlib.h>
#include <string.h>

int hash_code(char *key, int n) {
    int size = strlen(key);
    int sum = 0;
    for (int i = 0; i < size; i++)
        sum += key[i];

    return (sum + size) % n;   
}

struct table_t *table_create(int n) {
    if (assert_error(
        n <= 0,
        "table_create",
        ERROR_SIZE
    )) return NULL;

    struct table_t* table = create_dynamic_memory(sizeof(struct table_t));
    if (assert_error(
        table == NULL,
        "table_create",
        ERROR_MALLOC
    )) return NULL;

    table->size = n;
    table->lists = create_dynamic_memory(sizeof(struct list_t*) * n);
    if (assert_error(
        table->lists == NULL,
        "table_create",
        ERROR_MALLOC
    )) {
        destroy_dynamic_memory(table);
        return NULL;
    }

    for (int i = 0; i < table->size; i++) {
        table->lists[i] = list_create();
        if (assert_error(
            table->lists[i] == NULL,
            "table_create",
            ERROR_MALLOC
        )) {
            // cleanup on failure
            for (int j = i - 1; j >= 0; j--)
                destroy_dynamic_memory(table->lists[j]);
            destroy_dynamic_memory(table->lists);
            destroy_dynamic_memory(table);
            return NULL;
        }
    }

    return table;
}

int table_destroy(struct table_t *table) {
    if (assert_error(
        table == NULL || table->lists == NULL,
        "table_destroy",
        ERROR_NULL_POINTER_REFERENCE
    )) return M_ERROR;

    if (assert_error(
        table->size <= 0,
        "table_destroy",
        ERROR_SIZE
    )) return M_ERROR;

    for (int i = 0; i < table->size; i++)
        if (list_destroy(table->lists[i]) == M_ERROR)
            return M_ERROR;

    destroy_dynamic_memory(table->lists);
    destroy_dynamic_memory(table);
    return M_OK;    
}

int table_put(struct table_t *table, char *key, struct data_t *value) {
    if (assert_error(
        table == NULL || table->lists == NULL 
        || key == NULL || value == NULL,
        "table_put",
        ERROR_NULL_POINTER_REFERENCE
    )) return ADD_ERROR;

    if (assert_error(
        table->size <= 0,
        "table_destroy",
        ERROR_SIZE
    )) return ADD_ERROR;

    struct list_t* hash_table_entry = table->lists[hash_code(key, table->size)];
    struct entry_t* entry = entry_copy_create(key, value);

    return list_add(hash_table_entry, entry) == ADD_ERROR ? M_ERROR : M_OK;
}

struct data_t *table_get(struct table_t *table, char *key) {
    if (assert_error(
        table == NULL || table->lists == NULL 
        || key == NULL,
        "table_get",
        ERROR_NULL_POINTER_REFERENCE
    )) return NULL; 

    if (assert_error(
        table->size <= 0,
        "table_get",
        ERROR_SIZE
    )) return NULL;

    struct list_t* hash_table_entry = table->lists[hash_code(key, table->size)];
    struct entry_t* entry = list_get(hash_table_entry, key);

    return (entry == NULL) ? NULL : data_dup(entry->value);
}

int table_remove(struct table_t *table, char *key) {
    if (assert_error(
        table == NULL || table->lists == NULL 
        || key == NULL,
        "table_get",
        ERROR_NULL_POINTER_REFERENCE
    )) return REMOVE_ERROR; 

    if (assert_error(
        table->size <= 0,
        "table_get",
        ERROR_SIZE
    )) return REMOVE_ERROR;

    struct list_t* hash_table_entry = table->lists[hash_code(key, table->size)];
    return list_remove(hash_table_entry, key);
}

int table_size(struct table_t *table) {
    if (assert_error(
        table == NULL || table->lists == NULL,
        "table_size",
        ERROR_NULL_POINTER_REFERENCE
    )) return M_ERROR;

    if (assert_error(
        table->size <= 0,
        "table_size",
        ERROR_SIZE
    )) return M_ERROR;

    int size = 0;
    for (int i = 0; i < table->size; i++) {
        int hash_table_entry_size = list_size(table->lists[i]);
        if (hash_table_entry_size == M_ERROR)
            return M_ERROR;
        size += hash_table_entry_size;
    }
    return size;
}

char **table_get_keys(struct table_t *table) {
    if (assert_error(
        table == NULL || table->lists == NULL,
        "table_get_keys",
        ERROR_NULL_POINTER_REFERENCE
    )) return NULL; 

    if (assert_error(
        table->size <= 0,
        "table_get_keys",
        ERROR_SIZE
    )) return NULL;

    int size = table_size(table);
    if (assert_error(
        size <= 0,
        "table_get_keys",
        ERROR_SIZE
    )) return NULL;

    char** array = create_dynamic_memory(sizeof(char*) * (size + 1));
    if (assert_error(
        array == NULL,
        "table_get_keys",
        ERROR_MALLOC
    )) return NULL;

    int index = 0;
    for (int i = 0; i < table->size; i++) {
        struct list_t* hash_table_entry = table->lists[i];
        char** hash_table_entry_keys = list_get_keys(hash_table_entry);
        if (hash_table_entry_keys == NULL) {
            list_free_keys(array);
            return NULL;
        }
        for (int j = 0; j < hash_table_entry->size; j++) {
            array[index] = hash_table_entry_keys[j];
            index++;
        }
        destroy_dynamic_memory(hash_table_entry_keys);

    }
    array[index] = NULL;
    return array;
}

int table_free_keys(char **keys) {
    return list_free_keys(keys);
}