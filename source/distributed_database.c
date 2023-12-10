#include "distributed_database.h"

#include "database.h"
#include "client_stub.h"
#include "utils.h"


#include <stdio.h>


void ddatabase_init(struct TableServerDistributedDatabase* ddb, int n_lists) {
    if (assert_error(
        ddb == NULL,
        "database_init",
        ERROR_NULL_POINTER_REFERENCE
    )) return;

    ddb->db = (struct TableServerDatabase*)create_dynamic_memory(sizeof(struct TableServerDatabase));
    database_init(ddb->db, n_lists);
}

void ddatabase_destroy(struct TableServerDistributedDatabase* ddb) {
    if (assert_error(
        ddb == NULL,
        "ddatabase_destroy",
        ERROR_NULL_POINTER_REFERENCE
    )) return;

    database_destroy(ddb->db);
    destroy_dynamic_memory(ddb->db);
    if (ddb->replica != NULL)
        rtable_disconnect(ddb->replica);
}

int ddb_table_put(struct TableServerDistributedDatabase* ddb, char *key, struct data_t *value) {
    if (assert_error(
        ddb == NULL || ddb->db == NULL,
        "ddb_table_put",
        ERROR_NULL_POINTER_REFERENCE
    )) return -1; 

    if (db_table_put(ddb->db, key, value) == 0 && ddb->replica != NULL) {
        // success. forward to remote table
        return rtable_put_with_data(ddb->replica, key, value);
    }
    return 0;
}

int ddb_table_remove(struct TableServerDistributedDatabase* ddb, char* key) {
    if (assert_error(
        ddb == NULL || ddb->db == NULL,
        "ddb_table_remove",
        ERROR_NULL_POINTER_REFERENCE
    )) return -1; 

    if (db_table_remove(ddb->db, key) == 0 && ddb->replica != NULL) {
        // success. forward to remote table
        return rtable_del(ddb->replica, key);
    }
    return 0; 
}

struct data_t* ddb_table_get(struct TableServerDistributedDatabase* ddb, char *key) {
    if (assert_error(
        ddb == NULL,
        "ddb_table_get",
        ERROR_NULL_POINTER_REFERENCE
    )) return NULL; 

    return db_table_get(ddb->db, key);
}

int ddb_table_size(struct TableServerDistributedDatabase* ddb) {
    if (assert_error(
        ddb == NULL,
        "ddb_table_size",
        ERROR_NULL_POINTER_REFERENCE
    )) return -1; 

    return db_table_size(ddb->db);
}

char** ddb_table_get_keys(struct TableServerDistributedDatabase* ddb) {
    if (assert_error(
        ddb == NULL,
        "ddb_table_get_keys",
        ERROR_NULL_POINTER_REFERENCE
    )) return NULL;

    return db_table_get_keys(ddb->db);
}