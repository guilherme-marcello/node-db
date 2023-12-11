#include "table.h"
#include "table-private.h"
#include "list-private.h"
#include "entry.h"
#include "entry-private.h"
#include "utils.h"

#include <stdlib.h>
#include <string.h>

int hash_code(char *key, int n) {
    // get size of the string and iterate over chars, adding them to sum
    int size = strlen(key);
    int sum = 0;
    for (int i = 0; i < size; i++)
        sum += key[i];

    // return (sum + size) mod n
    return (sum + size) % n;   
}

struct table_t *table_create(int n) {
    if (assert_error(
        n <= 0,
        "table_create",
        ERROR_SIZE
    )) return NULL;

    // allocate memory for table
    struct table_t* table = create_dynamic_memory(sizeof(struct table_t));
    if (assert_error(
        table == NULL,
        "table_create",
        ERROR_MALLOC
    )) return NULL;

    // update table values, allocating memory for the lists
    table->size = n;
    table->lists = create_dynamic_memory(sizeof(struct list_t*) * n);
    if (assert_error(
        table->lists == NULL,
        "table_create",
        ERROR_MALLOC
    )) {
        // destroy table in case or allocation error
        destroy_dynamic_memory(table);
        return NULL;
    }

    // iterate over table, creating a new list for each index
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

    // iterate over table entries (lists) and destroy them
    for (int i = 0; i < table->size; i++)
        if (list_destroy(table->lists[i]) == M_ERROR)
            return M_ERROR;

    // destroy array of lists and table itself
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

    // use hash code to get which list from hashtable should be updated
    struct list_t* hash_table_entry = table->lists[hash_code(key, table->size)];
    // create new entry, by passing key-value to be copied 
    struct entry_t* entry = entry_copy_create(key, value);

    // try to add copied entry to list, returning M_ERROR in case of error. M_OK otherwise
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

    // use hash code to get which list from hashtable should be consulted
    struct list_t* hash_table_entry = table->lists[hash_code(key, table->size)];
    // get key from list
    struct entry_t* entry = list_get(hash_table_entry, key);

    // if the entry was not found, return null, copy the entry value otherwise
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

    // use hash code to get which list from hashtable should be updated
    struct list_t* hash_table_entry = table->lists[hash_code(key, table->size)];
    // return status of removing the given key from hash_table_entry
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

    // iterate over all hashtable entries (lists) and sum the size
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

    // retrive table size
    int size = table_size(table);
    if (assert_error(
        size < 0,
        "table_get_keys",
        ERROR_SIZE
    )) return NULL;

    // allocate memory to store all keys from the table
    char** array = create_dynamic_memory(sizeof(char*) * (size + 1));
    if (assert_error(
        array == NULL,
        "table_get_keys",
        ERROR_MALLOC
    )) return NULL;

    // iterate over all hashtable entries (lists - indexed by i)
    int index = 0;
    for (int i = 0; i < table->size; i++) {
        struct list_t* hash_table_entry = table->lists[i];
        // retrive entry's keys
        char** hash_table_entry_keys = list_get_keys(hash_table_entry);
        // iterate over keys in hashtable entry and copy them to buffer
        for (int j = 0; j < hash_table_entry->size; j++) {
            array[index] = hash_table_entry_keys[j];
            index++;
        }
        // destroy copied keys
        destroy_dynamic_memory(hash_table_entry_keys);

    }
    // set last element of array to NULL
    array[index] = NULL;
    return array;
}

int table_free_keys(char **keys) {
    return list_free_keys(keys);
}