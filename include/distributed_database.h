#ifndef _DDATABASE_H
#define _DDATABASE_H /* Distributed Database Module */

#include "database.h"
#include "client_stub.h"

struct TableServerDistributedDatabase {
    struct TableServerDatabase* db;
    struct rtable_t* replica; // remote table to receive forwarded requests
};

/**
 * @brief Initializes the distributed database.
 * 
 * @param ddb The distributed database to be initialized.
 * @param n_lists Number of lists in the local database.
 */
void ddatabase_init(struct TableServerDistributedDatabase* ddb, int n_lists);

/**
 * @brief Destroys the distributed database, freeing associated resources.
 * 
 * @param ddb The distributed database to be destroyed.
 */
void ddatabase_destroy(struct TableServerDistributedDatabase* ddb);

/**
 * @brief Inserts a key-value pair into the distributed database, forwarding the operation to the remote table if available.
 * 
 * @param ddb The distributed database.
 * @param key The key.
 * @param value The value.
 * @return 0 on success, -1 on failure.
 */
int ddb_table_put(struct TableServerDistributedDatabase* ddb, char* key, struct data_t* value);

/**
 * @brief Removes the entry with the given key from the distributed database, forwarding the operation to the remote table if available.
 * 
 * @param ddb The distributed database.
 * @param key The key to be removed.
 * @return 0 on success, -1 on failure.
 */
int ddb_table_remove(struct TableServerDistributedDatabase* ddb, char* key);

/**
 * @brief Retrieves the value associated with the given key from the distributed database.
 * 
 * @param ddb The distributed database.
 * @param key The key.
 * @return The associated value, or NULL if the key is not found.
 */
struct data_t* ddb_table_get(struct TableServerDistributedDatabase* ddb, char* key);

/**
 * @brief Retrieves the number of entries in the distributed database.
 * 
 * @param ddb The distributed database.
 * @return The number of entries, or -1 on failure.
 */
int ddb_table_size(struct TableServerDistributedDatabase* ddb);

/**
 * @brief Retrieves an array of keys from the distributed database.
 * 
 * @param ddb The distributed database.
 * @return An array of keys, or NULL on failure.
 */
char** ddb_table_get_keys(struct TableServerDistributedDatabase* ddb);

#endif
