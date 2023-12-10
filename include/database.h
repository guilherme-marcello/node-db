#ifndef _DATABASE_H
#define _DATABASE_H /* Módulo Database */

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

void database_init(struct TableServerDatabase* db, int n_lists);
void database_destroy(struct TableServerDatabase* db);

int db_migrate_table(struct TableServerDatabase* db, struct rtable_t* migration_table);

void db_decrement_active_clients(struct TableServerDatabase* db);
void db_increment_active_clients(struct TableServerDatabase* db);
void db_increment_op_counter(struct TableServerDatabase* db);
void db_add_to_computed_time(struct TableServerDatabase* db, long long delta);

int db_table_put(struct TableServerDatabase* db, char *key, struct data_t *value);
struct data_t* db_table_get(struct TableServerDatabase* db, char *key);
int db_table_remove(struct TableServerDatabase* db, char* key);
int db_table_size(struct TableServerDatabase* db);
char** db_table_get_keys(struct TableServerDatabase* db);

#endif