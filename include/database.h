#ifndef _DATABASE_H
#define _DATABASE_H /* Database Module */

#include "table.h"
#include "stats.h"
#include "client_stub.h"

#include <pthread.h>

struct TableServerDatabase {
    struct table_t* table;
    pthread_mutex_t table_mutex;

    struct statistics_t* stats;
    pthread_mutex_t op_counter_mutex;
    pthread_mutex_t computed_time_mutex;
    pthread_mutex_t active_mutex;

    pthread_attr_t thread_attr;
};

/**
 * @brief Initializes the database.
 * 
 * @param db The database to be initialized.
 * @param n_lists Number of lists in the table.
 */
void database_init(struct TableServerDatabase* db, int n_lists);

/**
 * @brief Destroys the database, freeing associated resources.
 * 
 * @param db The database to be destroyed.
 */
void database_destroy(struct TableServerDatabase* db);

/**
 * @brief Migrates the table entries from a remote table to the local database.
 * 
 * @param db The local database.
 * @param migration_table The remote table to migrate from.
 * @return 0 on success, -1 on failure.
 */
int db_migrate_table(struct TableServerDatabase* db, struct rtable_t* migration_table);

/**
 * @brief Decrements the count of active clients in the database.
 * 
 * @param db The database.
 */
void db_decrement_active_clients(struct TableServerDatabase* db);

/**
 * @brief Increments the count of active clients in the database.
 * 
 * @param db The database.
 */
void db_increment_active_clients(struct TableServerDatabase* db);

/**
 * @brief Increments the operation counter in the database.
 * 
 * @param db The database.
 */
void db_increment_op_counter(struct TableServerDatabase* db);

/**
 * @brief Adds the given time delta to the total computed time in the database.
 * 
 * @param db The database.
 * @param delta The time delta to add.
 */
void db_add_to_computed_time(struct TableServerDatabase* db, long long delta);

/**
 * @brief Inserts a key-value pair into the database table.
 * 
 * @param db The database.
 * @param key The key.
 * @param value The value.
 * @return 0 on success, -1 on failure.
 */
int db_table_put(struct TableServerDatabase* db, char* key, struct data_t* value);

/**
 * @brief Retrieves the value associated with the given key from the database table.
 * 
 * @param db The database.
 * @param key The key.
 * @return The associated value, or NULL if the key is not found.
 */
struct data_t* db_table_get(struct TableServerDatabase* db, char* key);

/**
 * @brief Removes the entry with the given key from the database table.
 * 
 * @param db The database.
 * @param key The key to be removed.
 * @return 0 on success, -1 on failure.
 */
int db_table_remove(struct TableServerDatabase* db, char* key);

/**
 * @brief Retrieves the number of entries in the database table.
 * 
 * @param db The database.
 * @return The number of entries, or -1 on failure.
 */
int db_table_size(struct TableServerDatabase* db);

/**
 * @brief Retrieves an array of keys from the database table.
 * 
 * @param db The database.
 * @return An array of keys, or NULL on failure.
 */
char** db_table_get_keys(struct TableServerDatabase* db);

#endif