#include "table.h"
#include "table-private.h"
#include "list-private.h"
#include "utils.h"

#include <stdlib.h>

int hash_code(char *key, int n) {
    int size = strlen(key);
    int sum = 0;
    for (int i = 0; i < size; i++)
        sum += key[i];

    return (sum + size) % n;   
}

struct table_t *table_create(int n) {
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
        table->lists[i] = create_dynamic_memory(sizeof(struct list_t));
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